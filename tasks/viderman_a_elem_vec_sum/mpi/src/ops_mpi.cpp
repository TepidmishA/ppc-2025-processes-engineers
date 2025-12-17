#include "viderman_a_elem_vec_sum/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cstddef>
#include <vector>

#include "viderman_a_elem_vec_sum/common/include/common.hpp"

namespace viderman_a_elem_vec_sum {

VidermanAElemVecSumMPI::VidermanAElemVecSumMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0.0;
}

bool VidermanAElemVecSumMPI::ValidationImpl() {
  return (GetOutput() == 0.0);
}

bool VidermanAElemVecSumMPI::PreProcessingImpl() {
  return true;
}

bool VidermanAElemVecSumMPI::RunImpl() {
  const auto &input_vector = GetInput();
  if (input_vector.empty()) {
    GetOutput() = 0.0;
    return true;
  }

  int int_rank = 0;
  int total_processes = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &int_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &total_processes);

  const auto my_rank = static_cast<size_t>(int_rank);
  const size_t element_count = input_vector.size();
  const auto total_procs_size = static_cast<size_t>(total_processes);

  const size_t base_chunk = element_count / total_procs_size;
  const size_t remaining_elements = element_count % total_procs_size;

  size_t my_chunk_size = base_chunk;
  if (my_rank < remaining_elements) {
    my_chunk_size = base_chunk + 1;
  }

  std::vector<double> local_data(my_chunk_size);
  std::vector<int> send_counts(total_procs_size);
  std::vector<int> displacements(total_procs_size);

  size_t displacement = 0;
  for (size_t i = 0; i < total_procs_size; ++i) {
    size_t chunk = base_chunk;
    if (i < remaining_elements) {
      chunk = base_chunk + 1;
    }
    send_counts[i] = static_cast<int>(chunk);
    displacements[i] = static_cast<int>(displacement);
    displacement += chunk;
  }

  MPI_Scatterv(input_vector.data(), send_counts.data(), displacements.data(), MPI_DOUBLE, local_data.data(),
               static_cast<int>(my_chunk_size), MPI_DOUBLE, 0, MPI_COMM_WORLD);

  double process_sum = 0.0;
  for (double value : local_data) {
    process_sum += value;
  }

  double final_result = 0.0;
  MPI_Allreduce(&process_sum, &final_result, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

  GetOutput() = final_result;
  return true;
}
bool VidermanAElemVecSumMPI::PostProcessingImpl() {
  return true;
}

}  // namespace viderman_a_elem_vec_sum
