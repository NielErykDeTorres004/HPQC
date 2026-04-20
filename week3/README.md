# Week 3 – MPI Hello World + Vector Sum

## Task 1 – Hello World with MPI

Wrote hello_mpi.c based on the example from the lecture notes, compiled with mpicc
and ran it with different numbers of processes. The output order changes between runs
which makes sense since there's no synchronisation between the print statements.

Also made a serial version (hello_mpi_serial.c) that takes a number as an argument
and just loops to fake the parallel output, compiled with regular gcc.

Timings with `time mpirun -np [N] bin/hello_mpi`:

| np | real (s) |
|----|----------|
| 2  | 0.347    |
| 4  | 0.403    |
| 8  | 0.406    |
| 16 | 0.458    |

With more processes user+sys starts to exceed real time, which is the parallel
execution showing up. Runtime stays around 0.35-0.45s regardless of np though —
pretty much all MPI startup overhead at this scale.

---

## Task 2 – proof.c breakdown

Went through proof.c to understand what it does. Basic structure:

Pseudocode:

    Read num_arg

    MPI_Init
    Get my_rank and uni_size

    if rank == 0:
        sum = 0
        for each other rank r:
            receive value from r
            add to sum
        print sum
    else:
        compute rank * num_arg
        send to rank 0

    MPI_Finalize

Each non-root process sends rank * num_arg to root, so you end up summing
num_arg * (1 + 2 + ... + P-1). That's just the triangle number formula, so the
whole thing is equivalent to:

    output = num_arg * (P - 1) * P / 2

where P is the number of processes.

---

## Task 3 – Vector Sum

### Serial version

vector_serial.c was provided and takes N as argument, allocates a vector,
initialises everything to zero and sums it (always prints 0 which isn't very useful).

Made a modified version (vector_serial_mod.c) that fills the vector with i % 100
before summing, so you actually get non-trivial results.

Timings on the modified serial version:

| N          | Sum         | real (s) |
|------------|-------------|----------|
| 1,000,000  | 49,500,000  | 0.019    |
| 10,000,000 | 495,000,000 | 0.107    |
| 20,000,000 | 990,000,000 | 0.205    |

Roughly linear scaling with N, as expected.

### MPI version

vector_mpi.c splits the vector across processes using MPI_Scatterv (handles the
case where N isn't evenly divisible), each rank sums its chunk, then MPI_Reduce
collects the totals.

Checked correctness first with small inputs (e.g. N=10 gives Sum=45, which is
0+1+...+9). Then tested performance at N=20,000,000:

| np | Sum         | real (s) |
|----|-------------|----------|
| 2  | 990,000,000 | 0.565    |
| 4  | 990,000,000 | 0.578    |
| 8  | 990,000,000 | 0.589    |

MPI is consistently slower than serial here. Even at smaller N (down to 10-1,000,000)
the MPI version sits around 0.38-0.45s while serial is much faster.

### What this shows

The problem is too lightweight for MPI to be worth it — summing integers is so fast
that startup and communication overhead completely dominates. More processes doesn't
help, it actually makes things slightly worse. Parallel isn't automatically faster; you
need enough actual computation to justify the overhead.
