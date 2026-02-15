#include <stdio.h>
#include <stdlib.h>
#include <time.h>

double to_second_float(struct timespec in_time);
struct timespec calculate_runtime(struct timespec start_time, struct timespec end_time);

int main(int argc, char **argv) 
{
	// creates and initialises the variables
	int i, input;
	i = input = 0;
	struct timespec start_time, end_time, time_diff;
	double runtime = 0.0;
	FILE *fp;
	char buffer[256];

	// checks if there are the right number of arguments
	if (argc == 2)
	{
		// converts the first argument to an integer
		input = atoi(argv[1]);
	}
	else
	{
		fprintf(stderr, "Incorrect arguments.  Usage: time_read [NUM]\n");
		exit(-1);
	}

	// open file for reading
	fp = fopen("data/time_print_output.txt", "r");
	if (fp == NULL)
	{
		perror("fopen");
		exit(-1);
	}
	
	// gets the time before the loop
	timespec_get(&start_time, TIME_UTC);

	// read entire file into buffer (loop until EOF)
	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		// do nothing, just read
	}

	fclose(fp);

	// gets the time after the loop
        timespec_get(&end_time, TIME_UTC);

	// calculates the runtime
	time_diff = calculate_runtime(start_time, end_time);
	runtime = to_second_float(time_diff);

	// outputs the runtime
	printf("\n\nRuntime for read loop: %lf seconds.\n\n", runtime);

	return 0;
}


double to_second_float(struct timespec in_time)
{
	float out_time = 0.0;
	long int seconds, nanoseconds;
	seconds = nanoseconds = 0;

	seconds = in_time.tv_sec;
	nanoseconds = in_time.tv_nsec;

	out_time = seconds + nanoseconds/1e9;

	return out_time;
}

struct timespec calculate_runtime(struct timespec start_time, struct timespec end_time)
{
	struct timespec time_diff;
	long int seconds, nanoseconds;
	seconds = nanoseconds = 0;

	seconds = end_time.tv_sec - start_time.tv_sec;
	nanoseconds = end_time.tv_nsec - start_time.tv_nsec;

	if (nanoseconds < 0)
	{
		seconds = seconds - 1;
		nanoseconds = ((long int) 1e9) - nanoseconds;
	}

	time_diff.tv_sec = seconds;
	time_diff.tv_nsec = nanoseconds;

	return time_diff;
}
