import sys
def main(): 
	output = 0

	# checks if there are the right number of arguments
        try:
		# converts the first argument to an integer
		in_arg = int(sys.argv[1]);
        except: # (argc != 2)
		# raises an error
		raise Exception("Incorrect arguments.  Usage: triangular [NUM]!")
		
	# iterates over all numbers up to AND INCLUDING the input
        for i in range(in_arg + 1):
		# adds the index to the output
		output = output + i
	
	# prints the output
	print("{}\n".format(output))

if __name__ == "__main__":
        main()
