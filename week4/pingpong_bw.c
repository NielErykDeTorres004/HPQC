#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// function declarations
void check_uni_size(int uni_size);
void check_args(int argc, char **argv, int *num_pings, int *message_bytes);
void root_task(int num_pings, int message_bytes);
void client_task(int message_bytes);

int main(int argc, char **argv)
{
        // declare and initialise error handling variable
        int ierror = 0;

        // declare and initialise arguments
        int num_pings = 0;
        int message_bytes = 0;
        check_args(argc, argv, &num_pings, &message_bytes);

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
                root_task(num_pings, message_bytes);
        }
        else // i.e. (1 == my_rank)
        {
                client_task(message_bytes);
        }

        // finalise MPI
        ierror = MPI_Finalize();
        return 0;
}

void check_args(int argc, char **argv, int *num_pings, int *message_bytes)
{
        // check the number of arguments
        if (argc != 3)
        {
                fprintf(stderr, "ERROR: Wrong number of arguments.\n");
                fprintf(stderr, "Correct use: mpirun -np 2 pingpong_bw [NUM_PINGS] [MESSAGE_BYTES]\n");
                exit(-1);
        }

        *num_pings = atoi(argv[1]);
        *message_bytes = atoi(argv[2]);

        if (*num_pings < 1)
        {
                fprintf(stderr, "ERROR: NUM_PINGS must be >= 1\n");
                exit(-1);
        }

        if (*message_bytes < 1)
        {
                fprintf(stderr, "ERROR: MESSAGE_BYTES must be >= 1\n");
                exit(-1);
        }
}

void check_uni_size(int uni_size)
{
        // sets the required universe size
        int required_uni_size = 2;

        if (uni_size == required_uni_size)
        {
                return;
        }
        else
        {
                fprintf(stderr, "ERROR: pingpong_bw must be run with exactly %d processes.\n", required_uni_size);
                fprintf(stderr, "MPI communicator size = %d\n", uni_size);
                MPI_Abort(MPI_COMM_WORLD, -1);
        }
}

void root_task(int num_pings, int message_bytes)
{
        // transmission variables
        int dest = 1;
        int source = 1;
        int tag_data = 0;
        int tag_stop = 1;
        MPI_Status status;

        // allocate and initialise the message buffer
        unsigned char *buffer = (unsigned char *)malloc((size_t)message_bytes);
        if (buffer == NULL)
        {
                fprintf(stderr, "ERROR: malloc failed for %d bytes\n", message_bytes);
                MPI_Abort(MPI_COMM_WORLD, -1);
        }

        for (int i = 0; i < message_bytes; i++)
        {
                buffer[i] = (unsigned char)(i % 256);
        }

        // get start_time
        double start_time = MPI_Wtime();

        // ping-pong loop
        for (int i = 0; i < num_pings; i++)
        {
                // ping: root sends buffer
                MPI_Send(buffer, message_bytes, MPI_BYTE, dest, tag_data, MPI_COMM_WORLD);

                // pong: root receives buffer back
                MPI_Recv(buffer, message_bytes, MPI_BYTE, source, tag_data, MPI_COMM_WORLD, &status);
        }

        // send stop signal (0-byte message with a different tag)
        MPI_Send(buffer, 0, MPI_BYTE, dest, tag_stop, MPI_COMM_WORLD);

        // get end_time
        double end_time = MPI_Wtime();

        double elapsed_time = end_time - start_time;
        double avg_time = elapsed_time / (double)num_pings;

        // bandwidth estimate: each ping-pong transfers 2 * message_bytes
        double bytes_per_pingpong = 2.0 * (double)message_bytes;
        double mib_per_s = (bytes_per_pingpong / avg_time) / (1024.0 * 1024.0);

        printf("Message size = %d bytes\n", message_bytes);
        printf("Num pings    = %d\n", num_pings);
        printf("Elapsed time = %.9f s\n", elapsed_time);
        printf("Avg time     = %.9f s per ping-pong\n", avg_time);
        printf("Bandwidth    = %.3f MiB/s (approx)\n", mib_per_s);

        free(buffer);
}

void client_task(int message_bytes)
{
        int dest = 0;
        int source = 0;
        int tag_data = 0;
        int tag_stop = 1;
        MPI_Status status;

        unsigned char *buffer = (unsigned char *)malloc((size_t)message_bytes);
        if (buffer == NULL)
        {
                fprintf(stderr, "ERROR: malloc failed for %d bytes\n", message_bytes);
                MPI_Abort(MPI_COMM_WORLD, -1);
        }

        while (1)
        {
                // receive either data or stop signal
                MPI_Recv(buffer, message_bytes, MPI_BYTE, source, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

                if (status.MPI_TAG == tag_stop)
                {
                        break;
                }

                // send buffer back
                MPI_Send(buffer, message_bytes, MPI_BYTE, dest, tag_data, MPI_COMM_WORLD);
        }

        free(buffer);
}
