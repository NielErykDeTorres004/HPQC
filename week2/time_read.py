import sys
import time

def main(): 
    # checks if there are the right number of arguments
    try:
        in_arg = int(sys.argv[1])
    except:
        raise Exception("Incorrect arguments.\nUsage: python time_read.py [NUM]")

    # open file for reading
    f = open("data/time_print_output.txt", "r")

    # gets the start time for the loop
    start_time = time.time()

    # read entire file into memory
    data = f.read()

    # gets the end time for the loop
    end_time = time.time()

    f.close()

    # gets the total time
    run_time = end_time - start_time

    # prints the runtime
    print("\n\nTime for read loop: {} seconds\n".format(run_time))

if __name__ == "__main__":
    main()
