# Week 2 – Performance and Parallelism

Programs written in both C and Python to benchmark execution time for different kinds of tasks. All timing was done on the Cheetah HPC cluster.

---

## Programs

### hello_world (.c / .py)
Basic hello world, just used as a sanity check and to compare startup overhead between C and Python.

### triangular (.c / .py)
Computes the nth triangular number (sum of 0 to n) using a loop. First thing written, mostly to check argument handling works.

### repeat_adder (.c / .py)
Multiplies two integers using repeated addition. Deliberately inefficient — the point is to see how runtime scales with the loop count argument. The second argument doesn't affect runtime, only the first does.

### time_print (.c / .py)
Prints integers up to n, with internal timing around the loop. Used to compare internal vs external timing and see how I/O to screen scales.

### time_write (.c / .py)
Same as time_print but writes to `data/time_print_output.txt` instead of the screen.

### time_read (.c / .py)
Reads back the file written by time_write and times how long it takes.

---

## How to compile and run

You need a `bin/` and `data/` directory in your home directory first:
```
mkdir -p ~/bin ~/data
```

Compile the C programs:
```
gcc week2/hello_world.c -o bin/hello_world
gcc week2/triangular.c -o bin/triangular
gcc week2/repeat_adder.c -o bin/repeat_adder
gcc week2/time_print.c -o bin/time_print
gcc week2/time_write.c -o bin/time_write
gcc week2/time_read.c -o bin/time_read
```

Run examples:
```
./bin/triangular 10
./bin/repeat_adder 100000000 2
time ./bin/time_print 1000000
time ./bin/time_write 10000
time ./bin/time_read 1
```

Python equivalents use the same arguments:
```
python3 week2/repeat_adder.py 100000000 2
python3 week2/time_print.py 1000000
```

Note: time_read expects the output file from time_write to already exist at `data/time_print_output.txt`.

---

## Results

### Hello World – startup overhead

| Language | real time |
|----------|-----------|
| C        | 0.005s    |
| Python   | 0.034s    |

Python takes about 7x longer just to start up. At this scale the actual computation is essentially nothing, it's all interpreter startup.

### Repeat Adder – loop scaling

Testing with the first argument varied (this is the loop count):

| Iterations | C (real) | Python (real) |
|------------|----------|----------------|
| 1,000,000  | 0.010s   | 0.081s         |
| 5,000,000  | 0.022s   | 0.279s         |
| 10,000,000 | 0.036s   | 0.527s         |
| 50,000,000 | 0.147s   | 2.348s         |
| 100,000,000| 0.262s   | 4.680s         |

Both scale roughly linearly with the number of iterations (O(n) as expected). C is about 17-18x faster at 100M iterations. The second argument has basically no effect on runtime since it's just used in the addition, not as a loop bound.

If you swap the arguments (e.g. `repeat_adder 2 100000000`) the C version finishes almost instantly since it only loops twice.

### Screen printing – internal vs external timing

10M iterations shown as a representative large case:

| Language | Internal time | External (real) |
|----------|--------------|-----------------|
| C        | 10.33s       | 10.33s          |
| Python   | 10.88s       | 10.91s          |

For large inputs, internal and external times are nearly identical. The loop dominates. For small inputs (100k), external time is noticeably larger due to process startup overhead. Both languages are similarly slow here since the bottleneck is writing to the terminal, not the interpreter.

Full print timing data:

| n         | C internal | C external | Python internal | Python external |
|-----------|------------|------------|-----------------|-----------------|
| 100,000   | 0.027s     | 0.032s     | 0.068s          | 0.101s          |
| 1,000,000 | 1.024s     | 0.980s     | 0.923s          | 0.957s          |
| 10,000,000| 10.329s    | 10.334s    | 10.879s         | 10.912s         |

(The C internal time being slightly higher than external for 1M is probably just measurement noise / OS scheduling.)

### File I/O

Write timing (output file was ~4.8KB at n=1000, ~58KB at n=10000):

| n      | C internal | C external | Python internal | Python external |
|--------|------------|------------|-----------------|-----------------|
| 1,000  | 0.000875s  | 0.007s     | 0.000592s       | 0.036s          |
| 10,000 | 0.003585s  | 0.009s     | 0.002842s       | 0.034s          |

Read timing (reading the 58KB file):

| Language | Internal       | External |
|----------|----------------|----------|
| C        | 0.000079s      | 0.007s   |
| Python   | 0.0000949s     | 0.035s   |

For file I/O, the internal times for both languages are very close — Python is not significantly slower once you strip out startup. The external times diverge a lot because Python's interpreter startup (~0.03s) completely swamps the actual read/write time.

---

## Conclusions

The main things I found:

- Both languages scale linearly for simple loops, which makes sense given the O(n) structure
- C is consistently faster for tight loops (roughly 17x at 100M iterations)
- For I/O-heavy tasks like printing to screen or writing files, the gap between languages narrows or disappears. The bottleneck shifts to the OS
- Internal timing isolates just the computation; external timing includes process startup. For small tasks the startup dominates, for large tasks they converge
- Python's startup overhead (~30ms) makes it look much worse than it is for fast tasks — the interpreter cost is basically fixed regardless of what the program does
