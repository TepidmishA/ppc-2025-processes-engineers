#include "olesnitskiy_v_find_viol/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <vector>

#include "olesnitskiy_v_find_viol/common/include/common.hpp"

namespace olesnitskiy_v_find_viol {

[[nodiscard]] int OlesnitskiyVFindViolMPI::CountViolation(double current, double next) {
  const double epsilon = 1e-10;
  return (current - next > epsilon) ? 1 : 0;
}

OlesnitskiyVFindViolMPI::OlesnitskiyVFindViolMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool OlesnitskiyVFindViolMPI::ValidationImpl() {
  return true;
}

bool OlesnitskiyVFindViolMPI::PreProcessingImpl() {
  return true;
}

bool OlesnitskiyVFindViolMPI::RunSequentialCase() {
  const auto &input_data = GetInput();
  int world_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  int viol = 0;
  if (world_rank == 0) {
    for (int i = 0; i < static_cast<int>(input_data.size()) - 1; i++) {
      viol += CountViolation(input_data[i], input_data[i + 1]);
    }
  }
  MPI_Bcast(&viol, 1, MPI_INT, 0, MPI_COMM_WORLD);
  GetOutput() = viol;
  return true;
}

bool OlesnitskiyVFindViolMPI::RunImpl() {
  if (GetInput().size() < 2) {
    GetOutput() = 0;
    return true;
  }

  const auto &input_data = GetInput();
  int world_size = 0;
  int world_rank = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  int total_size = static_cast<int>(GetInput().size());

  if (total_size <= world_size) {
    return RunSequentialCase();
  }
  int base_chunk = total_size / world_size;
  int remainder = total_size % world_size;
  std::vector<int> send_counts(world_size);
  std::vector<int> displacements(world_size);
  int displacement = 0;
  for (int i = 0; i < world_size; i++) {
    send_counts[i] = base_chunk + (i < remainder ? 1 : 0);
    displacements[i] = displacement;
    displacement += send_counts[i];
  }
  int my_chunk_size = send_counts[world_rank];
  std::vector<double> local_data(my_chunk_size);
  MPI_Scatterv(input_data.data(), send_counts.data(), displacements.data(), MPI_DOUBLE, local_data.data(),
               my_chunk_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  std::vector<double> prev_elements(world_size, 0.0);
  if (world_rank == 0) {
    for (int i = 1; i < world_size; i++) {
      int prev_block_end = displacements[i] - 1;
      prev_elements[i] = input_data[prev_block_end];
    }
  }
  double my_prev_element = 0.0;
  MPI_Scatter(prev_elements.data(), 1, MPI_DOUBLE, &my_prev_element, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  int local_viol = 0;
  if (world_rank > 0) {
    local_viol += CountViolation(my_prev_element, local_data[0]);
  }
  for (int i = 0; i < my_chunk_size - 1; i++) {
    local_viol += CountViolation(local_data[i], local_data[i + 1]);
  }
  int total_viol = 0;
  MPI_Allreduce(&local_viol, &total_viol, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

  GetOutput() = total_viol;
  return true;
}

bool OlesnitskiyVFindViolMPI::PostProcessingImpl() {
  return true;
}

}  // namespace olesnitskiy_v_find_viol
