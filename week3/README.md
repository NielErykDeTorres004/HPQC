Week 3 – MPI and Vector Sum

Task 1 – MPI Hello World
hello_mpi.c was compiled and run with multiple processes. Output order was inconsistent between runs, as expected.
Timings:
np	real (s)
2	0.347
4	0.403
8	0.406
16	0.458
As process count increases, user + sys exceeds real, indicating parallel execution. However, total runtime remains ~0.35–0.45 s, showing significant MPI startup overhead.
________________________________________
Task 2 – proof.c pseudocode and formula
Pseudocode
Read num_arg

MPI_Init
Get my_rank and uni_size

IF my_rank == 0:
    sum = 0
    FOR each rank r = 1..uni_size-1:
        receive value from r
        sum += value
    print sum
ELSE:
    value = my_rank * num_arg
    send value to rank 0

MPI_Finalize

Each client sends rank * num_arg to root.
So the entire program is equivalent to:
output = num_arg * (P - 1) * P / 2
where Pis the number of MPI processes.
________________________________________
Task 3 – Vector sum
vector_serial.c pseudocode

Read N
Allocate vector of size N
Initialise all values to 0
Sum all elements
Print sum
Free memory

Modified serial version
The vector was filled using:
vector[i] = i % 100
to create meaningful non-zero data.

Serial timings (modified)
N	Sum	real (s)
1,000,000	49,500,000	0.019
10,000,000	495,000,000	0.107
20,000,000	990,000,000	0.205

Runtime scales approximately linearly with N.

MPI vector sum (vector_mpi.c)
Correctness was verified (e.g. N=10 → Sum=45).
For N = 20,000,000:
np	Sum	real (s)
2	990,000,000	0.565
4	990,000,000	0.578
8	990,000,000	0.589

Additional tests with smaller N (10 to 1,000,000) also showed MPI runtimes around ~0.38–0.45 s, while serial runtimes were much smaller.
________________________________________

Conclusion
For this workload, MPI is consistently slower than serial. The problem has very little computation per element so MPI startup and communication overhead dominate runtime. Increasing the number of processes does not improve performance on this system.
This demonstrates that parallel execution does not automatically lead to speedup and overhead must be justified by sufficient computational work.
