#include "perepelkin_i_string_diff_char_count/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include "perepelkin_i_string_diff_char_count/common/include/common.hpp"

namespace perepelkin_i_string_diff_char_count {

PerepelkinIStringDiffCharCountMPI::PerepelkinIStringDiffCharCountMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool PerepelkinIStringDiffCharCountMPI::ValidationImpl() {
  return (GetOutput() == 0);
}

bool PerepelkinIStringDiffCharCountMPI::PreProcessingImpl() {
  return true;
}

bool PerepelkinIStringDiffCharCountMPI::RunImpl() {
  int ProcRank, ProcNum;
  MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
  MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);

  const auto& [s1, s2] = GetInput();
  const int len1 = static_cast<int>(s1.size());
  const int len2 = static_cast<int>(s2.size());
  const int min_len = std::min(len1, len2);
  const int max_len = std::max(len1, len2);

  // Prepare distribution of indices across processes
  int distribution_size = min_len / ProcNum;
  int remainder = min_len % ProcNum;
  int local_size = distribution_size + (ProcRank < remainder ? 1 : 0);

  std::vector<char> local_s1(local_size);
  std::vector<char> local_s2(local_size);

  std::vector<int> counts(ProcNum);
  std::vector<int> displacements(ProcNum);
  int offset = 0;
  for (int i = 0; i < ProcNum; i++) {
    counts[i] = distribution_size + (i < remainder ? 1 : 0);
    displacements[i] = offset;
    offset += counts[i];
  }

  // Scatter parts of the strings to processes
  MPI_Scatterv(s1.data(), counts.data(), displacements.data(), MPI_CHAR,
               local_s1.data(), local_size, MPI_CHAR, 0, MPI_COMM_WORLD);
  MPI_Scatterv(s2.data(), counts.data(), displacements.data(), MPI_CHAR,
               local_s2.data(), local_size, MPI_CHAR, 0, MPI_COMM_WORLD);

  // Compute local number of differing characters
  int local_diff = 0;
  for (int i = 0; i < local_size; ++i) {
    if (local_s1[i] != local_s2[i]) {
      local_diff++;
    }
  }

  // Reduce (sum) differences for the common parts
  int global_diff = 0;
  MPI_Allreduce(&local_diff, &global_diff, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  GetOutput() = global_diff + (max_len - min_len);
  return true;
}

bool PerepelkinIStringDiffCharCountMPI::PostProcessingImpl() {
  return true;
}

}  // namespace perepelkin_i_string_diff_char_count
