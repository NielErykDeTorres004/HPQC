import sys
import time

def main(): 
    # checks if there are the right number of arguments
    try:
        # converts the argument to integer
        in_arg = int(sys.argv[1])
    except:
        raise Exception("Incorrect arguments.\nUsage: python time_write.py [NUM]\ne.g.\npython time_write.py 23")

    # open file in data directory
    f = open("data/time_print_output.txt", "w")

    # gets the start time for the loop
    start_time = time.time()

    # iterates over all numbers up to the input
    for i in range(in_arg):
        # writes the index to file instead of printing
        f.write("{}, ".format(i))

    # gets the end time for the loop
    end_time = time.time()

    f.close()

    # gets the total time
    run_time = end_time - start_time

    # prints the runtime
    print("\n\nTime for write loop: {} seconds\n".format(run_time))

if __name__ == "__main__":
    main()
