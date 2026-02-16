#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int check_args(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s [VECTOR_SIZE]\n", argv[0]);
        exit(-1);
    }
    return atoi(argv[1]);
}

// calculate the expected sum on root (for correctness testing)
long long expected_sum(int N)
{
    long long sum = 0;
    for (int i = 0; i < N; i++)
        sum += (i % 100);
    return sum;
}

int main(int argc, char **argv)
{
    int my_rank = 0, uni_size = 0;
    int N = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &uni_size);

    // Root reads N, then broadcasts it so all ranks know the size
    if (my_rank == 0)
        N = check_args(argc, argv);

    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Decide how many elements each rank gets (balanced, remainder distributed to first ranks)
    int base = N / uni_size;
    int rem  = N % uni_size;

    int local_n = base + (my_rank < rem ? 1 : 0);

    // Compute starting index for this rank
    int start = my_rank * base + (my_rank < rem ? my_rank : rem);

    // Allocate full vector on every rank (broadcast requires all ranks to receive it)
    int *vec = (int*)malloc(N * sizeof(int));
    if (!vec)
    {
        fprintf(stderr, "Rank %d: malloc failed for vec\n", my_rank);
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    // Root initialises the full vector
    if (my_rank == 0)
    {
        for (int i = 0; i < N; i++)
            vec[i] = i % 100;  // meaningful deterministic data
    }

    // synchronise before benchmarking
    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    // Broadcast full vector to all ranks
    MPI_Bcast(vec, N, MPI_INT, 0, MPI_COMM_WORLD);

    // Local sum over this rank's portion
    long long local_sum = 0;
    for (int i = 0; i < local_n; i++)
        local_sum += vec[start + i];

    // Reduce sums to root
    long long global_sum = 0;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();
    double elapsed_time = end_time - start_time;

    // Root prints correctness check and time
    if (my_rank == 0)
    {
        long long exp = expected_sum(N);
        printf("Sum: %lld (expected %lld)\n", global_sum, exp);
        printf("Time: %.9f s\n", elapsed_time);
    }

    // Cleanup
    free(vec);

    MPI_Finalize();
    return 0;
}
