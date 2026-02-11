#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) 
{
	// creates and initialises the variables
	int i, input, output;
	i = input = output = 0;

	// checks if there are the right number of arguments
	if (argc == 2)
	{
		// converts the first argument to an integer
		input = atoi(argv[1]);
	}
	else //(argc != 2)
	{
		// raises an error
		fprintf(stderr, "Incorrect arguments.  Usage: triangular [NUM]!");
		// and crashes out
		exit(-1);
	}
	
	// iterates over all numbers up to AND INCLUDING the input
	for (i = 0; i <= input; i++)
	{
		// adds the index to the output
		output = output + i;
	}

	// prints the output
	printf("%d\n", output);
	return 0;
}
