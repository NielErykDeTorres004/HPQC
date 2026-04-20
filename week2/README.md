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

| Iterations  | C (real) | Python (real) |
|-------------|----------|---------------|
| 1,000,000   | 0.010s   | 0.081s        |
| 5,000,000   | 0.022s   | 0.279s        |
| 10,000,000  | 0.036s   | 0.527s        |
| 50,000,000  | 0.147s   | 2.348s        |
| 100,000,000 | 0.262s   | 4.680s        |

Both scale linearly which makes sense since its just a loop. C being ~17x faster than python at 100M was a bigger gap than I expected tbh. The second argument has no real effect on runtime — swapping them (e.g. `repeat_adder 2 100000000`) makes C finish almost instantly since it only loops twice.

### Screen printing – internal vs external timing

| n          | C internal | C external | Python internal | Python external |
|------------|------------|------------|-----------------|-----------------|
| 100,000    | 0.027s     | 0.032s     | 0.068s          | 0.101s          |
| 1,000,000  | 1.024s     | 0.980s     | 0.923s          | 0.957s          |
| 10,000,000 | 10.329s    | 10.334s    | 10.879s         | 10.912s         |

Both languages end up similarly slow here which surprised me — the bottleneck is just writing to the terminal so the language doesnt really matter. The C internal time at 1M coming out slightly higher than external is weird, probably just OS scheduling noise.

For large n internal and external times converge, for small n external is noticeably higher because process startup overhead dominates.

### File I/O

Write timing:

| n      | C internal | C external | Python internal | Python external |
|--------|------------|------------|-----------------|-----------------|
| 1,000  | 0.000875s  | 0.007s     | 0.000592s       | 0.036s          |
| 10,000 | 0.003585s  | 0.009s     | 0.002842s       | 0.034s          |

Read timing (reading the ~58KB file produced at n=10000):

| Language | Internal  | External |
|----------|-----------|----------|
| C        | 0.000079s | 0.007s   |
| Python   | 0.000095s | 0.035s   |

This was the most interesting result. Python looks about 4x slower externally but internally the two are nearly identical. Almost all of that difference is just interpreter startup (~0.03s) which is basically fixed regardless of what the program actually does. So Python isnt really slower at file I/O, it just looks that way when you time the whole process from outside.

---

## Conclusions

C wins on raw compute by a significant margin (~17x for tight loops) but that gap mostly disappears for I/O tasks where the OS is the bottleneck. The internal vs external timing comparison was useful for separating out Python's startup cost from its actual execution speed — a lot of the "Python is slow" reputation probably comes from benchmarking short programs where the interpreter startup dominates rather than the computation itself.

The screen printing results were probably the most unexpected — I went in assuming C would be faster there too but both languages hit the same wall since theyre both just waiting on the terminal. The 1M C internal > external result is a bit odd and I dont have a great explanation for it beyond measurement noise.

One thing to note is all of this was run on Cheetah and some of the shorter timings especially will vary a lot depending on system load at the time, so theyre not super reproducible.
