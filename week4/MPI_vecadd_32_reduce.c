#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// function declarations
int check_args(int argc, char **argv);
long long expected_sum(int N);

int main(int argc, char **argv)
{
    // declare and initialise error handling variable
    int ierror = 0;

    // declare and initialise rank and size variables
    int my_rank, uni_size;
    my_rank = uni_size = 0;

    // declare vector size
    int N = 0;

    // initialise MPI
    ierror = MPI_Init(&argc, &argv);

    // get rank and communicator size
    ierror = MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    ierror = MPI_Comm_size(MPI_COMM_WORLD, &uni_size);

    // root reads vector size
    if (0 == my_rank)
        N = check_args(argc, argv);

    // broadcast vector size to all ranks
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // calculate base chunk size and remainder
    int base = N / uni_size;
    int rem = N % uni_size;

    // determine local vector size
    int local_n = base + (my_rank < rem ? 1 : 0);

    // declare root-only variables
    int *sendcounts = NULL;
    int *displs = NULL;
    int *vec = NULL;

    if (0 == my_rank)
    {
        // allocate arrays for scatter
        sendcounts = (int*)malloc(uni_size * sizeof(int));
        displs = (int*)malloc(uni_size * sizeof(int));

        // calculate displacements
        int offset = 0;
        for (int r = 0; r < uni_size; r++)
        {
            sendcounts[r] = base + (r < rem ? 1 : 0);
            displs[r] = offset;
            offset += sendcounts[r];
        } // end for

        // allocate and initialise vector
        vec = (int*)malloc(N * sizeof(int));
        for (int i = 0; i < N; i++)
            vec[i] = i % 100;
    } // end if (0 == my_rank)

    // allocate local vector
    int *local_vec = (int*)malloc(local_n * sizeof(int));

    // synchronise before timing
    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    // scatter vector to all ranks
    MPI_Scatterv(vec, sendcounts, displs, MPI_INT,
                 local_vec, local_n, MPI_INT,
                 0, MPI_COMM_WORLD);

    // compute local sum
    long long local_sum = 0;
    for (int i = 0; i < local_n; i++)
        local_sum += local_vec[i];

    // reduce local sums to root
    long long global_sum = 0;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();
    double elapsed_time = end_time - start_time;

    // root prints result and timing
    if (0 == my_rank)
    {
        printf("Sum: %lld (expected %lld)\n", global_sum, expected_sum(N));
        printf("Time: %.9f s\n", elapsed_time);
    }

    // free memory
    free(local_vec);

    if (0 == my_rank)
    {
        free(vec);
        free(sendcounts);
        free(displs);
    }

    // finalise MPI
    ierror = MPI_Finalize();
    return 0;
}

int check_args(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s [VECTOR_SIZE]\n", argv[0]);
        exit(-1);
    }
    return atoi(argv[1]);
}

long long expected_sum(int N)
{
    long long sum = 0;
    for (int i = 0; i < N; i++)
        sum += (i % 100);
    return sum;
}
