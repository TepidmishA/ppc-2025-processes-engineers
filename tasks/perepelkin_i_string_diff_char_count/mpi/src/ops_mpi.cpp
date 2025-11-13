#include "perepelkin_i_string_diff_char_count/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <numeric>

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

  const size_t base_size = min_len / proc_num;
  const int remainder = static_cast<int>(min_len % proc_num);

  // Calculate local range for this process
  size_t local_start = 0;
  size_t local_end = 0;
  for (int i = 0; i < proc_num; i++) {
    size_t current_size = base_size + (i < remainder ? 1 : 0);
    if (i == proc_rank) {
      local_start = local_end;
      local_end = local_start + current_size;
      break;
    }
    local_end += current_size;
  }

  // Compute local number of differing characters
  auto s1_start = s1.begin() + static_cast<std::string::difference_type>(local_start);
  auto s1_end = s1.begin() + static_cast<std::string::difference_type>(local_end);
  auto s2_start = s2.begin() + static_cast<std::string::difference_type>(local_start);
  int local_diff = std::transform_reduce(s1_start, s1_end, s2_start, 0, std::plus<>(), std::not_equal_to<>());

  // Reduce (sum) differences for the common parts
  int global_diff = 0;
  MPI_Request request = MPI_REQUEST_NULL;
  MPI_Iallreduce(&local_diff, &global_diff, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD, &request);
  MPI_Wait(&request, MPI_STATUS_IGNORE);

  GetOutput() = global_diff + static_cast<int>(max_len - min_len);
  return true;
}

bool PerepelkinIStringDiffCharCountMPI::PostProcessingImpl() {
  return true;
}

}  // namespace perepelkin_i_string_diff_char_count
