#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

// function declarations
void root_task(int uni_size);
void client_task(int my_rank, int uni_size);
void check_uni_size(int uni_size);
void check_task(int uni_size, int my_rank);

int main(int argc, char **argv) 
{
	// declare and initialise error handling variable
	int ierror = 0;
	
	// declare and initialise rank and size varibles
	int my_rank, uni_size;
	my_rank = uni_size = 0;

	// intitalise MPI
	ierror = MPI_Init(&argc, &argv);

	// gets the rank and world size
	ierror = MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
	ierror = MPI_Comm_size(MPI_COMM_WORLD,&uni_size);

	// create and attach MPI buffer for buffered sends
	int buffer_size = (MPI_BSEND_OVERHEAD + sizeof(int)) * 2;
	void *bsend_buffer = malloc(buffer_size);
	MPI_Buffer_attach(bsend_buffer, buffer_size);


	check_uni_size(uni_size);
	check_task(uni_size, my_rank);


	// detach and free MPI buffer
	void *detached_buffer;
	int detached_size;
	MPI_Buffer_detach(&detached_buffer, &detached_size);
	free(detached_buffer);

	// finalise MPI
	ierror = MPI_Finalize();
	return 0;
}

void root_task(int uni_size)
{
	// creates and initialies transmission variables
	int recv_message, count, source, tag;
	recv_message = source = tag = 0;
	count = 1;
	MPI_Status status;

	// iterates through all the other ranks
	for (int their_rank = 1; their_rank < uni_size; their_rank++)
	{
		// sets the source argument to the rank of the sender
		source = their_rank;

		// internal timing for receive
		double start_time = MPI_Wtime();

		// receives the messages
		MPI_Recv(&recv_message, count, MPI_INT, source, tag, MPI_COMM_WORLD, &status);

		double end_time = MPI_Wtime();
		double elapsed_time = end_time - start_time;

		// prints the message from the sender
		printf("Hello, I am %d of %d. Received %d from Rank %d. Recv took %.9f s\n",
				0, uni_size, recv_message, source, elapsed_time);

	} // end for (int their_rank = 1; their_rank < uni_size; their_rank++)
}

void client_task(int my_rank, int uni_size)
{
	// creates and initialies transmission variables
	int send_message, count, dest, tag;
	send_message = dest = tag = 0;
	count = 1;

	// sets the destination for the message
	dest = 0; // destination is root

	// creates the message
	send_message = my_rank * 10;

	// internal timing for send
	double start_time = MPI_Wtime();

	// sends the message
	MPI_Bsend(&send_message, count, MPI_INT, dest, tag, MPI_COMM_WORLD);

	double end_time = MPI_Wtime();
	double elapsed_time = end_time - start_time;

	// prints the message from the sender
	printf("Hello, I am %d of %d. Sent %d to Rank %d. Send took %.9f s\n",
			my_rank, uni_size, send_message, dest, elapsed_time);
}

void check_uni_size(int uni_size)
{
	// sets the minimum universe size
	int min_uni_size = 2;

	// checks there are sufficient tasks to communicate with
	if (uni_size >= min_uni_size)
	{
		return;
	}
	else
	{
		printf("Unable to communicate with less than %d processes. MPI communicator size = %d\n",
		       min_uni_size, uni_size);
		MPI_Abort(MPI_COMM_WORLD, -1);
	}
}

void check_task(int uni_size, int my_rank)
{
	if (0 == my_rank)
	{
		root_task(uni_size);
	}
	else
	{
		client_task(my_rank, uni_size);
	}
}
