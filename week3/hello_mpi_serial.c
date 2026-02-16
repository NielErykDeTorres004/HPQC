#include <stdio.h>
// #include <mpi.h>
#include <stdlib.h>

int check_args(int argc, char **argv);

int main(int argc, char **argv) 
{
	// declare and initialise error handling variable
	// int ierror = 0;
	
	// declare and initialise rank and size varibles
	int my_rank, uni_size;
	my_rank = uni_size = 0;

	// intitalise MPI
	// ierror = MPI_Init(&argc, &argv);

	// gets the rank and world size
	// ierror = MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
	// ierror = MPI_Comm_size(MPI_COMM_WORLD,&uni_size);
	uni_size = check_args(argc, argv);

	for (my_rank = 0; my_rank < uni_size; my_rank++)
	{
		// prints to the screen
		printf("Hello, I am %d of %d\n",my_rank, uni_size);
	}

	// finalise MPI
	// ierror = MPI_Finalize();
	return 0;
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
                fprintf(stderr, "Correct use: hello_mpi_serial [NUMBER]\n");

                // and exit COMPLETELY
                exit (-1);
        }
        return num_arg;
}
