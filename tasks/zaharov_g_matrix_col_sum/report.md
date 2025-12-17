# Matrix Column Sum

- **Student:** Zaharov Gleb Mihajlovič, group 3823Б1ПР4
- **Technology:** SEQ | MPI
- **Variant:** 12


## 1. Introduction

Matrix operations are fundamental in scientific computing and data analysis.
Computing column sums is a common preprocessing step for normalization,
aggregation, and statistical analysis. For large matrices (e.g., 10'000×10'000),
sequential computation becomes prohibitively slow. This project implements a
parallel solution using MPI (Message Passing Interface) to accelerate column
sum calculations while maintaining numerical correctness.


## 2. Problem Statement

**Task:** Given a matrix A of size M×N with real-valued elements, compute vector
S of length N where each element is the sum of the corresponding matrix column:

$$ s_j = \sum_{i=1}^{M} a_{ij} $$

**Input/Output:**
- **Input:** Matrix as `std::vector<std::vector<double>>`
- **Output:** Vector as `std::vector<double>`
- **Constraints:**
  - All rows must have equal length
  - Matrix may be empty, square, or rectangular


## 3. Baseline Algorithm (Sequential)

The sequential algorithm implements straightforward nested loops:

```cpp
OutType out(GetInput()[0].size());

for (size_t i = 0; i < GetInput()[0].size(); i++) {
  double sum = 0;

  for (const auto &row : GetInput()) {
    sum += row[i];
  }

  out[i] = sum;
}
```
src: [ops_seq.cpp#L36-L46](https://github.com/VerySweetBread/ppc-2025-processes-engineers/blob/zaharov_g_matrix_col_sum/tasks/zaharov_g_matrix_col_sum/seq/src/ops_seq.cpp#L36-L46)

**Time Complexity:** O(M×N)
**Space Complexity:** O(N) for the result vector


## 4. Parallelization Scheme (MPI)

### Data Distribution
The matrix is partitioned by columns among MPI processes using a balanced
distribution algorithm. Each process receives a contiguous column interval to
process independently.

### Communication Pattern
**Master-Worker pattern with rank 0 as coordinator:**
1. **Distribution Phase:** Process 0 calculates intervals for all processes
using `CalcInterval()` and sends `[start, end]` pairs via `MPI_Send`
2. **Computation Phase:** Each process computes partial sums for its assigned columns
3. **Collection Phase:** Workers send results back to process 0 using `MPI_Send/MPI_Recv`
4. **Broadcast Phase:** Process 0 assembles final vector and broadcasts to all
processes via `MPI_Bcast`

### Key MPI Operations
- `MPI_Send/MPI_Recv` for point-to-point communication
- `MPI_Bcast` for result synchronization
- `MPI_Comm_size/MPI_Comm_rank` for process management

### Load Balancing Algorithm
```cpp
std::vector<int> CalsInterval(int thread_amount, int rank, int column_amount) {
  int base_chunk = column_amount / thread_amount;
  int remainder = column_amount % thread_amount;

  int start = (rank * base_chunk) + std::min(rank, remainder);
  int end = start + base_chunk + (rank < remainder ? 1 : 0);

  return {start, end};
}
```
src: [ops_mpi.cpp#L111-L119](https://github.com/VerySweetBread/ppc-2025-processes-engineers/blob/zaharov_g_matrix_col_sum/tasks/zaharov_g_matrix_col_sum/mpi/src/ops_mpi.cpp#L111-L119)

## 5. Implementation Details

### Code Structure
```
tasks/zaharov_g_matrix_col_sum/
├── mpi/
│   ├── include/ops_mpi.hpp      # MPI interface
│   └── src/ops_mpi.cpp          # MPI implementation
├── seq/
│   ├── include/ops_seq.hpp      # Sequential interface
│   └── src/ops_seq.cpp          # Sequential implementation
└── tests/                       # Test suites
```

### Key Functions
1. `ValidationImpl()` - Checks matrix consistency (all rows equal length)
2. `PreProcessingImpl()` - Initializes output data structures
3. `RunImpl()` - Core computation logic

### Memory Considerations
- Each process stores only its assigned columns during computation
- Final result replicated on all processes via broadcast


## 6. Experimental Setup

### Hardware/Software Environment
- **CPU:** Intel Core i5-10600KF (6 cores, 12 threads, 4.1GHz base)
- **RAM:** 32GB
- **OS:** NixOS (Linux-based distribution)
- **Compiler:** GCC 14.3
- **Build Type:** Release

### Test Configuration
- **Matrix Size:** 10'000 × 10'000 elements (generated)
- **Number of Processes:** 6 (matching physical cores)


## 7. Results and Discussion

### 7.1 Correctness Verification
Correctness was verified through:
- **Unit Tests:** 8 functional tests covering edge cases (1×N, M×1 matrices)
- **Comparison Testing:** MPI results matched sequential implementation within ±1 error

### 7.2 Performance Analysis

#### Execution Times

| Mode     | Implementation | Time (s) | Speedup |
|----------|----------------|----------|---------|
| Pipeline | SEQ            | 0.936    | 1.00    |
| Task Run | SEQ            | 1.053    | 0.89    |
| Pipeline | MPI            | 0.206    | 4.54    |
| Task Run | MPI            | 0.195    | 5.40    |

#### Key Observations:
1. **MPI Acceleration:** x4-5 speedup over sequential implementation
2. **Efficiency:** 76-90% parallel efficiency on 6 processes

#### Scalability Considerations:
- **Strong Scaling:** Good efficiency for tested configuration
- **Limitations:** Performance limited by column-wise memory access pattern
- **Bottlenecks:** Communication overhead becomes significant for small per-process workloads
