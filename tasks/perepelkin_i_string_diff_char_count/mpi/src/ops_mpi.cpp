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
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank_);
  MPI_Comm_size(MPI_COMM_WORLD, &proc_num_);

  SetTypeOfTask(GetStaticTypeOfTask());
  if (proc_rank_ == 0) {
    GetInput() = in;
  }
  GetOutput() = 0;
}

bool PerepelkinIStringDiffCharCountMPI::ValidationImpl() {
  return (GetOutput() == 0);
}

bool PerepelkinIStringDiffCharCountMPI::PreProcessingImpl() {
  return true;
}

bool PerepelkinIStringDiffCharCountMPI::RunImpl() {
  size_t min_len = 0;
  size_t max_len = 0;

  if (proc_rank_ == 0) {
    const auto &[s1, s2] = GetInput();
    min_len = std::min(s1.size(), s2.size());
    max_len = std::max(s1.size(), s2.size());
  }

  MPI_Bcast(&min_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&max_len, 1, MPI_INT, 0, MPI_COMM_WORLD);

  std::vector<char> local_s1;
  std::vector<char> local_s2;
  DistributeData(min_len, local_s1, local_s2);

  int local_diff = std::transform_reduce(local_s1.begin(), local_s1.end(), local_s2.begin(), 0, std::plus<>(),
                                         std::not_equal_to<>());

  int global_diff = 0;
  MPI_Allreduce(&local_diff, &global_diff, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

  GetOutput() = global_diff + static_cast<int>(max_len - min_len);
  return true;
}

void PerepelkinIStringDiffCharCountMPI::DistributeData(size_t min_len, std::vector<char> &local_s1,
                                                       std::vector<char> &local_s2) {
  const int base_size = static_cast<int>(min_len / proc_num_);
  const int remainder = static_cast<int>(min_len % proc_num_);

  std::vector<int> counts(proc_num_);
  std::vector<int> displacements(proc_num_);

  if (proc_rank_ == 0) {
    for (int i = 0, offset = 0; i < proc_num_; i++) {
      counts[i] = base_size + (i < remainder ? 1 : 0);
      displacements[i] = offset;
      offset += counts[i];
    }
  }

  const int local_size = base_size + (proc_rank_ < remainder ? 1 : 0);
  local_s1.resize(local_size);
  local_s2.resize(local_size);

  const char *s1_data = nullptr;
  const char *s2_data = nullptr;
  if (proc_rank_ == 0) {
    const auto &[s1, s2] = GetInput();
    s1_data = s1.data();
    s2_data = s2.data();
  }

  MPI_Scatterv(s1_data, counts.data(), displacements.data(), MPI_CHAR, local_s1.data(), local_size, MPI_CHAR, 0,
               MPI_COMM_WORLD);
  MPI_Scatterv(s2_data, counts.data(), displacements.data(), MPI_CHAR, local_s2.data(), local_size, MPI_CHAR, 0,
               MPI_COMM_WORLD);
}

bool PerepelkinIStringDiffCharCountMPI::PostProcessingImpl() {
  return true;
}

}  // namespace perepelkin_i_string_diff_char_count
