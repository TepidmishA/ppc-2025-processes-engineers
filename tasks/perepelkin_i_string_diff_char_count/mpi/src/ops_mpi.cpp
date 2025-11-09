#include "perepelkin_i_string_diff_char_count/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <numeric>
#include <vector>

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
  int proc_rank = 0;
  int proc_num = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &proc_num);

  const auto &[s1, s2] = GetInput();
  const size_t len1 = s1.size();
  const size_t len2 = s2.size();
  const size_t min_len = std::min(len1, len2);
  const size_t max_len = std::max(len1, len2);

  // Prepare distribution of indices across processes
  const size_t base_size = min_len / proc_num;
  const int remainder = static_cast<int>(min_len % proc_num);
  std::vector<int> counts(proc_num);
  std::vector<int> displacements(proc_num);
  for (int i = 0, offset = 0; i < proc_num; i++) {
    counts[i] = static_cast<int>(base_size + (i < remainder ? 1 : 0));
    displacements[i] = offset;
    offset += counts[i];
  }

  // Allocate local buffers
  const int local_size = counts[proc_rank];
  std::vector<char> local_s1(local_size);
  std::vector<char> local_s2(local_size);

  // Scatter parts of the strings to processes
  MPI_Scatterv(s1.data(), counts.data(), displacements.data(), MPI_CHAR, local_s1.data(), local_size, MPI_CHAR, 0,
               MPI_COMM_WORLD);
  MPI_Scatterv(s2.data(), counts.data(), displacements.data(), MPI_CHAR, local_s2.data(), local_size, MPI_CHAR, 0,
               MPI_COMM_WORLD);

  // Compute local number of differing characters
  int local_diff = std::transform_reduce(local_s1.begin(), local_s1.end(), local_s2.begin(), 0, std::plus<>(),
                                         std::not_equal_to<>());

  // Reduce (sum) differences for the common parts
  int global_diff = 0;
  MPI_Allreduce(&local_diff, &global_diff, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

  GetOutput() = global_diff + static_cast<int>(max_len - min_len);
  return true;
}

bool PerepelkinIStringDiffCharCountMPI::PostProcessingImpl() {
  return true;
}

}  // namespace perepelkin_i_string_diff_char_count
