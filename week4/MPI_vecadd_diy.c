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

    // Allocate local buffer on every rank
    int *local_vec = (int*)malloc(local_n * sizeof(int));
    if (!local_vec)
    {
        fprintf(stderr, "Rank %d: malloc failed for local_vec\n", my_rank);
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    // Root will allocate and initialise full vector, then send chunks to other ranks
    int *vec = NULL;

    // synchronise before benchmarking
    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    if (my_rank == 0)
    {
        // Allocate and fill full vector on root
        vec = (int*)malloc(N * sizeof(int));
        if (!vec)
        {
            fprintf(stderr, "Root: malloc failed for vec\n");
            MPI_Abort(MPI_COMM_WORLD, -1);
        }

        for (int i = 0; i < N; i++)
            vec[i] = i % 100;  // meaningful deterministic data

        // Copy root's own chunk into local buffer
        for (int i = 0; i < local_n; i++)
            local_vec[i] = vec[start + i];

        // Send chunks to other ranks using MPI_Send
        for (int r = 1; r < uni_size; r++)
        {
            int r_local_n = base + (r < rem ? 1 : 0);
            int r_start   = r * base + (r < rem ? r : rem);

            MPI_Send(vec + r_start, r_local_n, MPI_INT, r, 0, MPI_COMM_WORLD);
        }
    }
    else
    {
        // Non-root ranks receive their chunk
        MPI_Status status;
        MPI_Recv(local_vec, local_n, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }

    // Local sum
    long long local_sum = 0;
    for (int i = 0; i < local_n; i++)
        local_sum += local_vec[i];

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
    free(local_vec);
    if (my_rank == 0)
        free(vec);

    MPI_Finalize();
    return 0;
}
