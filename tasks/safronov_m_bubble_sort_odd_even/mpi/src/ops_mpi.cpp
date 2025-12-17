#include "safronov_m_bubble_sort_odd_even/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <vector>

#include "safronov_m_bubble_sort_odd_even/common/include/common.hpp"

namespace safronov_m_bubble_sort_odd_even {

SafronovMBubbleSortOddEvenMPI::SafronovMBubbleSortOddEvenMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
}

bool SafronovMBubbleSortOddEvenMPI::ValidationImpl() {
  return GetOutput().empty();
}

bool SafronovMBubbleSortOddEvenMPI::PreProcessingImpl() {
  GetOutput().clear();
  return true;
}

void SafronovMBubbleSortOddEvenMPI::SendingVector(int rank) {
  int size_vec = 0;
  if (rank == 0) {
    size_vec = static_cast<int>(GetInput().size());
  }
  MPI_Bcast(&size_vec, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if (rank != 0) {
    GetInput().resize(size_vec);
  }
  MPI_Bcast(GetInput().data(), size_vec, MPI_INT, 0, MPI_COMM_WORLD);
}

std::vector<int> SafronovMBubbleSortOddEvenMPI::CalculatingInterval(int size_prcs, int rank, int size_arr) {
  std::vector<int> vec(2);
  int whole_part = size_arr / size_prcs;
  int real_part = size_arr % size_prcs;
  int start = rank * whole_part;
  if ((rank - 1 < real_part) && (rank - 1 != -1)) {
    start += rank;
  } else if (rank != 0) {
    start += real_part;
  }
  int end = start + whole_part - 1;
  if (rank < real_part) {
    end += 1;
  }
  vec[0] = start;
  vec[1] = end;
  return vec;
}

void SafronovMBubbleSortOddEvenMPI::OddEvenBubble(std::vector<int> &own_data, int own_size, int begin, int phase) {
  if (own_size == 0) {
    return;
  }
  int indent = begin % 2 == phase ? 0 : 1;
  for (int j = 0 + indent; j < own_size - 1; j += 2) {
    if (own_data[j] > own_data[j + 1]) {
      int tmp = own_data[j + 1];
      own_data[j + 1] = own_data[j];
      own_data[j] = tmp;
    }
  }
}

void SafronovMBubbleSortOddEvenMPI::DataExchange(std::vector<int> &own_data, int rank, int neighbor) {
  MPI_Status status;
  int elem_send = neighbor == 1 ? own_data.back() : own_data[0];
  int elem_recv = 0;
  MPI_Sendrecv(&elem_send, 1, MPI_INT, rank + neighbor, 0, &elem_recv, 1, MPI_INT, rank + neighbor, 0, MPI_COMM_WORLD,
               &status);
  if (neighbor == 1) {
    int min = std::min(elem_send, elem_recv);
    own_data.back() = min;
  } else {
    int max = std::max(elem_send, elem_recv);
    own_data[0] = max;
  }
}

void SafronovMBubbleSortOddEvenMPI::EvenPhase(std::vector<int> &own_data, std::vector<int> &interval, int size_arr,
                                              int rank, int size) {
  int own_size = static_cast<int>(own_data.size());
  OddEvenBubble(own_data, own_size, interval[0], 0);
  if ((rank % 2 == 0) && (rank + 1 < size) && (interval[1] != size_arr - 1) && (interval[0] <= interval[1])) {
    DataExchange(own_data, rank, 1);
  } else if ((interval[0] <= interval[1]) && (rank % 2 == 1)) {
    DataExchange(own_data, rank, -1);
  }
}

void SafronovMBubbleSortOddEvenMPI::OddPhase(std::vector<int> &own_data, std::vector<int> &interval, int size_arr,
                                             int rank, int size) {
  int own_size = static_cast<int>(own_data.size());
  OddEvenBubble(own_data, own_size, interval[0], 1);
  if ((rank % 2 == 1) && (rank + 1 < size) && (interval[1] != size_arr - 1) && (interval[0] <= interval[1])) {
    DataExchange(own_data, rank, 1);
  } else if ((interval[0] <= interval[1]) && (rank != 0) && (rank % 2 == 0)) {
    DataExchange(own_data, rank, -1);
  }
}

void SafronovMBubbleSortOddEvenMPI::BasisSortingLocalArrays(std::vector<int> &own_data, std::vector<int> &interval,
                                                            int size_arr, int rank, int size) {
  for (int i = 0; i < size_arr + size - 1; i++) {
    if (i % 2 == 0) {
      EvenPhase(own_data, interval, size_arr, rank, size);
    } else {
      OddPhase(own_data, interval, size_arr, rank, size);
    }
  }
}

void SafronovMBubbleSortOddEvenMPI::SendingResult(int rank) {
  int total_size = 0;
  if (rank == 0) {
    total_size = static_cast<int>(GetOutput().size());
  }

  MPI_Bcast(&total_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    GetOutput().resize(total_size);
  }

  MPI_Bcast(GetOutput().data(), total_size, MPI_INT, 0, MPI_COMM_WORLD);
}

bool SafronovMBubbleSortOddEvenMPI::RunImpl() {
  int size = 0;
  int rank = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  SendingVector(rank);

  if (rank == 0) {
    int size_arr = static_cast<int>(GetInput().size());
    std::vector<int> sizes_local_arrays(size);
    for (int i = 1; i < size; i++) {
      std::vector<int> interval = CalculatingInterval(size, i, size_arr);
      sizes_local_arrays[i] = (interval[1] + 1) - interval[0];
      MPI_Send(interval.data(), 2, MPI_INT, i, 0, MPI_COMM_WORLD);
    }
    std::vector<int> interval = CalculatingInterval(size, 0, size_arr);
    std::vector<int> own_data = {};
    if (interval[0] <= interval[1]) {
      own_data = std::vector<int>(GetInput().begin() + interval[0], GetInput().begin() + interval[1] + 1);
    }
    BasisSortingLocalArrays(own_data, interval, size_arr, rank, size);
    GetOutput().insert(GetOutput().end(), own_data.begin(), own_data.end());
    MPI_Status status;
    for (int i = 1; i < size; i++) {
      std::vector<int> buf(sizes_local_arrays[i]);
      MPI_Recv(buf.data(), sizes_local_arrays[i], MPI_INT, i, 2, MPI_COMM_WORLD, &status);
      GetOutput().insert(GetOutput().end(), buf.begin(), buf.end());
    }
  } else {
    MPI_Status status;
    std::vector<int> buf(2);
    MPI_Recv(buf.data(), 2, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    std::vector<int> own_data = {};
    if (buf[0] <= buf[1]) {
      own_data = std::vector<int>(GetInput().begin() + buf[0], GetInput().begin() + buf[1] + 1);
    }
    int size_arr = static_cast<int>(GetInput().size());
    BasisSortingLocalArrays(own_data, buf, size_arr, rank, size);
    MPI_Send(own_data.data(), static_cast<int>(own_data.size()), MPI_INT, 0, 2, MPI_COMM_WORLD);
  }
  SendingResult(rank);
  return true;
}

bool SafronovMBubbleSortOddEvenMPI::PostProcessingImpl() {
  return true;
}

}  // namespace safronov_m_bubble_sort_odd_even
