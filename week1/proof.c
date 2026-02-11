#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// function declarations
int root_task(int uni_size);
void client_task(int my_rank, int num_arg);
int check_args(int argc, char **argv);
void check_uni_size(int uni_size);
void check_task(int uni_size, int my_rank, int num_arg);

int main(int argc, char **argv) 
{
	// declare and initialise error handling variable
	int ierror = 0;
	
	// declare and initialise the numerical argument variable
	int num_arg = check_args(argc, argv);

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

	// checks what task to do and does it
	check_task(uni_size, my_rank, num_arg);

	// finalise MPI
	ierror = MPI_Finalize();
	return 0;
}

int root_task(int uni_size)
{

	// creates and initialies transmission variables
	int recv_message, count, source, tag;
	recv_message = source = tag = 0;
	count = 1;
	MPI_Status status;

	// creates and intiialises the variable for the final output
	int output_sum = 0;
	
	// iterates through all the other ranks
	for (source = 1; source < uni_size; source++)
	{
		// receives the messages
		MPI_Recv(&recv_message, count, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
		// adds the values to a running tally
		output_sum += recv_message;
	} // end for (source = 1; source < uni_size; source++)

	// outputs and returns the result
	printf("The combined result is %d\n", output_sum);
	return output_sum;
}

void client_task(int my_rank, int num_arg)
{
	// creates and initialies transmission variables
	int send_message, count, dest, tag;
	send_message = dest = tag = 0;
	count = 1;

	// sets the destination for the message
	dest = 0; // destination is root

	// creates the message
	send_message = my_rank * num_arg;

	// sends the message
	MPI_Send(&send_message, count, MPI_INT, dest, tag, MPI_COMM_WORLD);
}

int check_args(int argc, char **argv)
{
	// declare and initialise the numerical argument
	int num_arg = 0;

	// check the number of arguments
	if (argc == 2) // program name and numerical argument
	{
		// declare and initialise the numerical argument
		num_arg = atoi(argv[1]);
	}
	else // the number of arguments is incorrect
	{
		// raise an error
		fprintf(stderr, "ERROR: You did not provide a numerical argument!\n");
		fprintf(stderr, "Correct use: mpicc -n 4 proof [NUMBER]\n");

		// and exit COMPLETELY
		exit (-1);
	}
	return num_arg;
}

void check_uni_size(int uni_size)
{
	// sets the minimum universe size
	int min_uni_size = 1;
	// checks there are sufficient tasks to communicate with
	if (uni_size >= min_uni_size)
	{
		return;
	} // end if (uni_size >= min_uni_size)
	else // i.e. uni_size < min_uni_size
	{
		// Raise an error
		fprintf(stderr, "Unable to communicate with fewer than %d processes.", min_uni_size);
		fprintf(stderr, "MPI communicator size = %d\n", uni_size);

		// and exit COMPLETELY
		exit(-1);
	}
}

void check_task(int uni_size, int my_rank, int num_arg)
{
	// checks which process is running and calls the appropriate task
	if (0 == my_rank)
	{
		root_task(uni_size);
	} // end if (0 == my_rank)
	else // i.e. (0 != my_rank)
	{
		client_task(my_rank, num_arg);
	} // end else // i.e. (0 != my_rank)
}
