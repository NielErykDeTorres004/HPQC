#include <stdio.h>
#include <mpi.h>

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

	// creates and initialies transmission variables
	int send_message, recv_message, count, dest, source, tag;
	send_message = recv_message = dest = source = tag = 0;
	count = 1;
	MPI_Status status;

	check_uni_size(uni_size);
	check_task(uni_size, my_rank);


	// finalise MPI
	ierror = MPI_Finalize();
	return 0;
}

void root_task(int uni_size)
{
}

void client_task(int my_rank, int uni_size)
{
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
}
