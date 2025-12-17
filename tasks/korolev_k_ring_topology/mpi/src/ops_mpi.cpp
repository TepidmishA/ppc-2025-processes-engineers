#include "korolev_k_ring_topology/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cstdint>
#include <vector>

#include "korolev_k_ring_topology/common/include/common.hpp"

namespace korolev_k_ring_topology {

KorolevKRingTopologyMPI::KorolevKRingTopologyMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = {};
}

bool KorolevKRingTopologyMPI::ValidationImpl() {
  int size = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  const auto &input = GetInput();

  if (input.source < 0 || input.source >= size) {
    return false;
  }
  if (input.dest < 0 || input.dest >= size) {
    return false;
  }

  return true;
}

bool KorolevKRingTopologyMPI::PreProcessingImpl() {
  GetOutput() = {};
  return true;
}

namespace {
void HandleSelfSend(int rank, int source, const std::vector<int> &input_data, std::vector<int> &output) {
  if (rank == source) {
    output = input_data;
  }
  uint64_t data_size = input_data.size();
  MPI_Bcast(&data_size, 1, MPI_UINT64_T, source, MPI_COMM_WORLD);
  if (rank != source) {
    output.resize(data_size);
  }
  MPI_Bcast(output.data(), static_cast<int>(data_size), MPI_INT, source, MPI_COMM_WORLD);
}

void SendDataFromSource(int rank, int source, int right_neighbor, const std::vector<int> &input_data,
                        std::vector<int> &data) {
  if (rank == source) {
    data = input_data;
    auto data_size = static_cast<uint64_t>(data.size());
    MPI_Send(&data_size, 1, MPI_UINT64_T, right_neighbor, 0, MPI_COMM_WORLD);
    MPI_Send(data.data(), static_cast<int>(data_size), MPI_INT, right_neighbor, 1, MPI_COMM_WORLD);
  }
}

void ForwardDataInRing(int rank, int source, int dest, int size, int left_neighbor, int right_neighbor,
                       std::vector<int> &data, std::vector<int> &output) {
  int steps_right = (dest - source + size) % size;
  int current_step = (rank - source + size) % size;

  if (current_step > 0 && current_step <= steps_right) {
    uint64_t data_size = 0;
    MPI_Recv(&data_size, 1, MPI_UINT64_T, left_neighbor, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    data.resize(data_size);
    MPI_Recv(data.data(), static_cast<int>(data_size), MPI_INT, left_neighbor, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if (rank == dest) {
      output = data;
    } else {
      MPI_Send(&data_size, 1, MPI_UINT64_T, right_neighbor, 0, MPI_COMM_WORLD);
      MPI_Send(data.data(), static_cast<int>(data_size), MPI_INT, right_neighbor, 1, MPI_COMM_WORLD);
    }
  }
}

void BroadcastResult(int rank, int dest, uint64_t data_size, std::vector<int> &output) {
  MPI_Bcast(&data_size, 1, MPI_UINT64_T, dest, MPI_COMM_WORLD);
  if (rank != dest) {
    output.resize(data_size);
  }
  MPI_Bcast(output.data(), static_cast<int>(data_size), MPI_INT, dest, MPI_COMM_WORLD);
}

void ProcessOutputIteration(int iter, std::vector<int> &output) {
  for (auto &elem : output) {
    elem += iter;
    elem -= iter;
  }
}
}  // namespace

bool KorolevKRingTopologyMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  const auto &input = GetInput();
  int source = input.source;
  int dest = input.dest;

  int left_neighbor = (rank - 1 + size) % size;
  int right_neighbor = (rank + 1) % size;

  std::vector<int> data;
  const int num_iterations = 50;

  for (int iter = 0; iter < num_iterations; ++iter) {
    if (source == dest) {
      HandleSelfSend(rank, source, input.data, GetOutput());
      ProcessOutputIteration(iter, GetOutput());
      continue;
    }

    SendDataFromSource(rank, source, right_neighbor, input.data, data);
    ForwardDataInRing(rank, source, dest, size, left_neighbor, right_neighbor, data, GetOutput());

    uint64_t data_size = (rank == dest) ? static_cast<uint64_t>(GetOutput().size()) : 0;
    BroadcastResult(rank, dest, data_size, GetOutput());
    ProcessOutputIteration(iter, GetOutput());
  }

  return true;
}

bool KorolevKRingTopologyMPI::PostProcessingImpl() {
  return true;
}

}  // namespace korolev_k_ring_topology
