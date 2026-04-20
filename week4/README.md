# Week 4 - MPI Communications

This week covers MPI point-to-point and collective communications. There are three parts: testing different send modes, ping-pong latency/bandwidth benchmarking and collective operations for vector addition.

---

## Part 1: Communication Testing

### Files
- `comm_test_mpi.c` - starting point from the demo materials, basic send/recv in one big main()
- `comm_test_mpi_func.c` - same thing refactored into functions (root_task, client_task etc.)
- `comm_test_ssend.c` - synchronous send variant
- `comm_test_bsend.c` - buffered send variant
- `comm_test_rsend.c` - ready send variant
- `comm_test_isend.c` - non-blocking send + wait variant

### Compiling and running
```bash
mpicc week4/comm_test_mpi_func.c -o bin/comm_test_mpi_func
mpirun -np 4 bin/comm_test_mpi_func
```
Same pattern for the other files. Needs at least 2 processes.

### Observations

Running with different numbers of processes shows that the order messages arrive at root varies each run. This is expected since MPI gives no ordering guarantees between independent processes.

**Ssend** blocks the sender until the receiver has posted its recv, so send times tend to be higher (~30-300µs) but the first recv on root is also longer since it has to wait. Once root starts receiving, subsequent recvs are nearly instant because the senders were already waiting.

**Bsend** returns almost immediately (~8-30µs) since it just copies to a local buffer. The actual transfer happens in the background.

**Isend+Wait** times out similarly to Bsend (~9-30µs). Since we call MPI_Wait immediately there's no real benefit over a normal send here. Non-blocking only helps if you have computation to overlap with the send.

**Rsend** technically requires the recv to already be posted before calling it, which isn't guaranteed in this program (root posts recvs one at a time, so later ranks may send before root is ready). It ran without errors on Cheetah but this is unsafe in general and would likely fail on other systems.

For reliability Ssend or standard Send is the better choice for this kind of program.

---

## Part 2: Ping-Pong Benchmarking

### Files
- `pingpong.c` - sends a counter back and forth to measure round-trip latency
- `pingpong_bw.c` - sends variable-size arrays to measure bandwidth
- `bw_results.txt` - raw output from the bandwidth sweep
- `bw_analysis.ipynb` - plots and linear fit for the bandwidth data

### Compiling and running
```bash
mpicc week4/pingpong.c -o bin/pingpong
mpicc week4/pingpong_bw.c -o bin/pingpong_bw

mpirun -np 2 bin/pingpong 10000
mpirun -np 2 bin/pingpong_bw 10000 1024
```
Both require exactly 2 processes.

### How pingpong works

Root sends a counter to the client (ping), client increments it and sends it back (pong). The whole loop is timed and divided by the number of pings to get average round-trip time. A sentinel value of -1 is sent at the end to stop the client. The incrementing also checks correctness. The final value should equal num_pings.

### Latency results

| Num pings | Avg per ping-pong |
|-----------|-------------------|
| 10 | 4.075 µs |
| 100 | 2.510 µs |
| 1,000 | 0.830 µs |
| 10,000 | 0.754 µs |
| 100,000 | 0.656 µs |
| 1,000,000 | 0.673 µs |

Converges around 0.65 to 0.75 µs with enough pings. With only 10 pings the startup overhead dominates and the estimate is unreliable. Since both processes are on the same node this is shared-memory MPI, so latency is lower than a real network would give.

### Bandwidth results

The pingpong_bw program sends a malloc'd byte array and the client sends it straight back. Bandwidth = (2 * message_bytes) / avg_time.

| Size | Bandwidth |
|------|-----------|
| 8 B | 26.6 MiB/s |
| 512 B | 637 MiB/s |
| 1 KiB | 1316 MiB/s |
| **4 KiB** | **854 MiB/s** |
| 32 KiB | 3684 MiB/s |
| 256 KiB | 6267 MiB/s |
| 1 MiB | 7577 MiB/s |

Full data in bw_results.txt. Plots in bw_plot_linear.png and bw_plot_fit.png.

Bandwidth generally increases with message size. For small messages latency overhead dominates. The notable drop at 4 KiB is probably where MPI switches from eager to rendezvous protocol (sender waits for receiver acknowledgement before sending). Above that it climbs again and levels off around 7 to 7.5 GiB/s.

A linear fit on the large-message region (>=16 KiB) gives:
- Latency = 8.16 µs
- Bandwidth = 7370 MiB/s

---

## Part 3: Collective Communications (Vector Addition)

All programs sum a vector where vec[i] = i % 100. Each prints the computed sum and expected sum for correctness checking. Test: N=10 gives 45, N=1000000 gives 49500000.

### Files
- `MPI_vector_addition.c` - week 3 starting point
- `MPI_vecadd_bcast.c` - broadcasts full vector, each rank sums its portion
- `MPI_vecadd_scatter.c` - scatters chunks with MPI_Scatterv
- `MPI_vecadd_diy.c` - root sends chunks manually with MPI_Send loops
- `MPI_vecadd_32_sendrecv.c` - collects partial sums with manual send/recv
- `MPI_vecadd_32_gather.c` - collects partial sums with MPI_Gather, root loops to add
- `MPI_vecadd_32_reduce.c` - MPI_Reduce with MPI_SUM
- `MPI_vecadd_33_customreduce.c` - compares MPI_SUM vs a custom reduce operation

The "32"/"33" prefix refers to Part 3 Step 2/3 of the exercise.

### Compiling and running
```bash
mpicc week4/MPI_vecadd_scatter.c -o bin/vecadd_scatter
mpirun -np 4 bin/vecadd_scatter 1000000
```
All programs take one argument: vector size N.

### Step 1 - Distribution strategies (N=1000000, 4 processes)

| Method | Time |
|--------|------|
| Scatter | 0.010395 s |
| Broadcast | 0.007705 s |
| DIY (manual sends) | 0.010525 s |

Expected broadcast to be slowest since it sends the whole array to every rank but it was actually fastest. Probably because MPI_Bcast on a shared-memory node uses an optimised path that avoids actually copying the data multiple times. Scatter and DIY are similar since they're doing essentially the same thing.

### Step 2 - Reduction strategies (N=1000000, 4 processes)

| Method | Time |
|--------|------|
| Manual send/recv | 0.003865 s |
| Gather + loop | 0.004045 s |
| MPI_Reduce | 0.004327 s |

Again the results were backwards from what was expected. MPI_Reduce was slowest and manual send/recv was fastest. For only 4 processes contributing 1 value each the overhead of the collective operations outweighs any benefit. A tree-reduction would only help at much larger process counts.

### Step 3 - Custom reduce

Implemented my_sum_op() which adds long long arrays element-wise, registered with MPI_Op_create(). Results across 5 runs:

| Run | MPI_SUM | Custom |
|-----|---------|--------|
| 1 | 6.3 µs | 1.9 µs |
| 2 | 7.1 µs | 2.6 µs |
| 3 | 15.9 µs | 3.5 µs |
| 4 | 4.5 µs | 3.1 µs |
| 5 | 10.3 µs | 1.0 µs |

Both give the correct answer every time. The custom op is consistently faster which is unexpected. MPI_SUM should have hardware-optimised paths. At this scale (1 element, 4 processes) the measurements are in the noise and the custom function call path probably just has less overhead than the built-in dispatch. At larger element counts MPI_SUM would likely win.
