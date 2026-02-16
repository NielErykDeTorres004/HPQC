#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// function declarations
int check_args(int argc, char **argv);
void check_uni_size(int uni_size);
void root_task(int num_pings);
void client_task(void);

int main(int argc, char **argv)
{
        // declare and initialise error handling variable
        int ierror = 0;

        // check the number of arguments
        int num_pings = check_args(argc, argv);

        // intitalise MPI
        ierror = MPI_Init(&argc, &argv);

        // declare and initialise rank and size varibles
        int my_rank, uni_size;
        my_rank = uni_size = 0;

        // gets the rank and world size
        ierror = MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        ierror = MPI_Comm_size(MPI_COMM_WORLD, &uni_size);

        // checks the universe size is correct
        check_uni_size(uni_size);

        // do the ping-pong tasks
        if (0 == my_rank)
        {
                root_task(num_pings);
        }
        else // i.e. (1 == my_rank)
        {
                client_task();
        }

        // finalise MPI
        ierror = MPI_Finalize();
        return 0;
}

int check_args(int argc, char **argv)
{
        // declare and initialise the numerical argument
        int num_pings = 0;

        // check the number of arguments
        if (argc == 2) // program name and numerical argument
        {
                num_pings = atoi(argv[1]);
        }
        else
        {
                // raise an error
                fprintf(stderr, "ERROR: You did not provide num_pings!\n");
                fprintf(stderr, "Correct use: mpirun -np 2 pingpong [NUM_PINGS]\n");
                exit(-1);
        }

        // basic validation
        if (num_pings < 1)
        {
                fprintf(stderr, "ERROR: num_pings must be >= 1\n");
                exit(-1);
        }

        return num_pings;
}

void check_uni_size(int uni_size)
{
        // sets the required universe size
        int required_uni_size = 2;

        // checks there are sufficient tasks to communicate with
        if (uni_size == required_uni_size)
        {
                return;
        }
        else
        {
                // Raise an error
                fprintf(stderr, "ERROR: pingpong must be run with exactly %d processes.\n", required_uni_size);
                fprintf(stderr, "MPI communicator size = %d\n", uni_size);

                // and exit COMPLETELY
                MPI_Abort(MPI_COMM_WORLD, -1);
        }
}

void root_task(int num_pings)
{
        // creates and initialies transmission variables
        int counter = 0;
        int count = 1;
        int dest = 1;
        int source = 1;
        int tag = 0;
        MPI_Status status;

        // get start_time
        double start_time = MPI_Wtime();

        // While loop with condition counter < num_pings
        while (counter < num_pings)
        {
                // Root process sends counter to client // ping
                MPI_Send(&counter, count, MPI_INT, dest, tag, MPI_COMM_WORLD);

                // Root receives counter from client // pong
                MPI_Recv(&counter, count, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
        }

        // get end_time
        double end_time = MPI_Wtime();

        // calculate elapsed_time and average_time
        double elapsed_time = end_time - start_time;
        double average_time = elapsed_time / (double)num_pings;

        // Root prints counter, elapsed_time and average_time
        printf("Final counter = %d\n", counter);
        printf("Elapsed time  = %.9f s\n", elapsed_time);
        printf("Average time  = %.9f s per ping-pong\n", average_time);
}

void client_task(void)
{
        // creates and initialies transmission variables
        int counter = 0;
        int count = 1;
        int dest = 0;
        int source = 0;
        int tag = 0;
        MPI_Status status;

        while (1)
        {
                // Client receives counter
                MPI_Recv(&counter, count, MPI_INT, source, tag, MPI_COMM_WORLD, &status);

                // Client increments counter by 1
                counter = counter + 1;

                // Client sends counter to Root // pong
                MPI_Send(&counter, count, MPI_INT, dest, tag, MPI_COMM_WORLD);

                // stop condition: once we have incremented to num_pings, root will exit
                // but client doesn't know num_pings here, so we just keep going until MPI_Finalize.
                // In practice, root will stop receiving and MPI will terminate the job cleanly.
                // (This is fine for this practical.)
        }
}
