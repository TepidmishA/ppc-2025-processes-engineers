#include "batkov_f_vector_sum/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cstddef>
#include <limits>
#include <vector>

#include "batkov_f_vector_sum/common/include/common.hpp"

namespace batkov_f_vector_sum {

BatkovFVectorSumMPI::BatkovFVectorSumMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());

  int rank = 0;
  int mpi_size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

  m_rank_ = static_cast<size_t>(rank);
  m_mpi_size_ = static_cast<size_t>(mpi_size);

  if (m_rank_ == 0) {
    GetInput() = in;
  } else {
    GetInput() = std::vector<int>();
  }

  GetOutput() = 0;
}

bool BatkovFVectorSumMPI::ValidationImpl() {
  return GetOutput() == 0;
}

bool BatkovFVectorSumMPI::PreProcessingImpl() {
  return true;
}

bool BatkovFVectorSumMPI::RunImpl() {
  size_t input_size = 0;
  if (m_rank_ == 0) {
    input_size = GetInput().size();
  }

  MPI_Bcast(&input_size, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);

  if (input_size == 0) {
    GetOutput() = 0;
    return true;
  }

  const size_t base_chunk = input_size / m_mpi_size_;
  const size_t remaining_elements = input_size % m_mpi_size_;

  size_t chunk_size = base_chunk;
  if (m_rank_ < remaining_elements) {
    chunk_size = base_chunk + 1;
  }
  std::vector<int> local_data(chunk_size);

  std::vector<int> send_counts(m_mpi_size_);
  std::vector<int> displs(m_mpi_size_);
  size_t disp = 0;

  for (size_t i = 0; i < m_mpi_size_; ++i) {
    size_t chunk = base_chunk;
    if (i < remaining_elements) {
      chunk = base_chunk + 1;
    }

    int int_chunk = static_cast<int>(chunk);
    int int_disp = static_cast<int>(disp);

    if (int_chunk > std::numeric_limits<int>::max() || int_disp > std::numeric_limits<int>::max()) {
      return false;
    }

    send_counts[i] = int_chunk;
    displs[i] = int_disp;
    disp += chunk;
  }

  MPI_Scatterv(GetInput().data(), send_counts.data(), displs.data(), MPI_INT, local_data.data(),
               static_cast<int>(chunk_size), MPI_INT, 0, MPI_COMM_WORLD);

  int local_sum = 0;
  for (int val : local_data) {
    local_sum += val;
  }

  int global_sum = 0;
  MPI_Allreduce(&local_sum, &global_sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

  GetOutput() = global_sum;
  return true;
}

bool BatkovFVectorSumMPI::PostProcessingImpl() {
  return true;
}

}  // namespace batkov_f_vector_sum
