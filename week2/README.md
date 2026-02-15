# Week 2
Week 2 – Performance Evaluation Summary
Repeat Adder
The repeat adder program multiplies two integers using repeated addition.
Runtime depends only on the argument controlling the loop count.
Testing increasing loop sizes showed approximately linear scaling (O(n)) in both C and Python.
For 100,000,000 iterations:
	C: ~0.26 s
	Python: ~2.35 s
Python is roughly 15–20× slower per iteration due to interpreter overhead. This demonstrates that algorithm structure determines scaling while language choice affects constant factors.
________________________________________
Internal vs External Timing
Using time_print, both internal timing (within the program) and external timing (Linux time) were compared.
For small inputs, external runtime was dominated by startup overhead.
For large inputs (10,000,000 prints), internal and external times converged (~10–11 s), showing that the loop dominated execution time.
This demonstrates that external timing measures total process cost, while internal timing isolates the computational section.
________________________________________
File I/O
Modified programs were used to write and read a ~58 KB file.
Write (10,000 lines)
	C internal: ~0.0036 s
	Python internal: ~0.0028 s
	External Python time significantly higher due to startup overhead.
Read (58 KB)
	C internal: ~0.00008 s
	Python internal: ~0.00009 s
	External Python runtime again dominated by interpreter startup.
Small file operations are fast internally, but total runtime is strongly affected by process startup cost.
________________________________________
Conclusions
	Both languages scale linearly for simple loops.
	C is significantly faster for tight loops.
	I/O operations dominate performance at large output sizes.
	External timing includes startup overhead, internal timing isolates computation.
	Python’s startup overhead significantly impacts small tasks.

