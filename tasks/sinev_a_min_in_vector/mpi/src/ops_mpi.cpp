#include "sinev_a_min_in_vector/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <limits>
#include <vector>

#include "sinev_a_min_in_vector/common/include/common.hpp"

namespace sinev_a_min_in_vector {

SinevAMinInVectorMPI::SinevAMinInVectorMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::numeric_limits<int>::max();
}

bool SinevAMinInVectorMPI::ValidationImpl() {
  int proc_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

  bool is_valid = true;

  if (proc_rank == 0) {
    is_valid = !GetInput().empty();
  }

  MPI_Bcast(&is_valid, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);

  return is_valid;
}

bool SinevAMinInVectorMPI::PreProcessingImpl() {
  return true;
}

bool SinevAMinInVectorMPI::RunImpl() {
  int proc_num = 0;
  int proc_rank = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &proc_num);
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

  std::vector<int> local_data;
  int global_size = 0;

  if (proc_rank == 0) {
    global_size = static_cast<int>(GetInput().size());
  }

  // Рассылаем размер всем процессам
  MPI_Bcast(&global_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (global_size == 0) {
    GetOutput() = std::numeric_limits<int>::max();
    return true;
  }

  int block_size = global_size / proc_num;
  int remainder = global_size % proc_num;

  int local_size = block_size + (proc_rank < remainder ? 1 : 0);
  local_data.resize(local_size);

  std::vector<int> sendcounts(proc_num);
  std::vector<int> displacements(proc_num);

  if (proc_rank == 0) {
    for (int i = 0; i < proc_num; i++) {
      sendcounts[i] = block_size + (i < remainder ? 1 : 0);
      displacements[i] = (i * block_size) + std::min(i, remainder);
    }
  }

  MPI_Bcast(sendcounts.data(), proc_num, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(displacements.data(), proc_num, MPI_INT, 0, MPI_COMM_WORLD);

  MPI_Scatterv(proc_rank == 0 ? GetInput().data() : nullptr, sendcounts.data(), displacements.data(), MPI_INT,
               local_data.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);

  int local_min = std::numeric_limits<int>::max();
  for (int value : local_data) {
    local_min = std::min(local_min, value);
  }

  int global_min = std::numeric_limits<int>::max();
  MPI_Allreduce(&local_min, &global_min, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);

  GetOutput() = global_min;
  return true;
}

bool SinevAMinInVectorMPI::PostProcessingImpl() {
  return true;
}

}  // namespace sinev_a_min_in_vector
