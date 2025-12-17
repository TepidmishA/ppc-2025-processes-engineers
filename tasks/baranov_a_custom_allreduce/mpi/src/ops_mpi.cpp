#include "baranov_a_custom_allreduce/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <variant>
#include <vector>

#include "baranov_a_custom_allreduce/common/include/common.hpp"

namespace baranov_a_custom_allreduce {

void BaranovACustomAllreduceMPI::TreeBroadcast(void *buffer, int count, MPI_Datatype datatype, MPI_Comm comm,
                                               int root) {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &size);

  if (count == 0) {
    return;
  }
  if (rank == root) {
    for (int i = 0; i < size; i++) {
      if (i != root) {
        MPI_Send(buffer, count, datatype, i, 0, comm);
      }
    }
  } else {
    MPI_Recv(buffer, count, datatype, root, 0, comm, MPI_STATUS_IGNORE);
  }
}

void BaranovACustomAllreduceMPI::TreeReduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op,
                                            MPI_Comm comm, int root) {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &size);

  if (count == 0) {
    return;
  }

  int type_size = 0;
  MPI_Type_size(datatype, &type_size);

  std::memcpy(recvbuf, sendbuf, static_cast<std::size_t>(count) * static_cast<std::size_t>(type_size));
  if (rank == root) {
    for (int i = 0; i < size; i++) {
      if (i != root) {
        std::vector<unsigned char> temp_buf(static_cast<std::size_t>(count) * static_cast<std::size_t>(type_size));
        if (temp_buf.empty()) {
          throw std::runtime_error("Memory allocation failed");
        }

        MPI_Recv(temp_buf.data(), count, datatype, i, 0, comm, MPI_STATUS_IGNORE);
        PerformOperation(temp_buf.data(), recvbuf, count, datatype, op);
      }
    }
  } else {
    MPI_Send(recvbuf, count, datatype, root, 0, comm);
    std::memset(recvbuf, 0, static_cast<std::size_t>(count) * static_cast<std::size_t>(type_size));
  }
}

void BaranovACustomAllreduceMPI::PerformOperation(void *inbuf, void *inoutbuf, int count, MPI_Datatype datatype,
                                                  MPI_Op op) {
  if (op != MPI_SUM) {
    throw std::runtime_error("Only MPI_SUM operation is supported");
  }

  if (datatype == MPI_INT) {
    auto *in = static_cast<int *>(inbuf);
    auto *inout = static_cast<int *>(inoutbuf);
    for (int i = 0; i < count; i++) {
      inout[i] += in[i];
    }
  } else if (datatype == MPI_FLOAT) {
    auto *in = static_cast<float *>(inbuf);
    auto *inout = static_cast<float *>(inoutbuf);
    for (int i = 0; i < count; i++) {
      inout[i] += in[i];
    }
  } else if (datatype == MPI_DOUBLE) {
    auto *in = static_cast<double *>(inbuf);
    auto *inout = static_cast<double *>(inoutbuf);
    for (int i = 0; i < count; i++) {
      inout[i] += in[i];
    }
  } else {
    throw std::runtime_error("Unsupported datatype");
  }
}

namespace {
void ProcessRootReceive(std::vector<unsigned char> &temp_buf, int count, MPI_Datatype datatype, MPI_Op op,
                        MPI_Comm comm, int root, int size, int type_size) {
  for (int i = 0; i < size; i++) {
    if (i != root) {
      std::vector<unsigned char> recv_buf(static_cast<std::size_t>(count) * static_cast<std::size_t>(type_size));
      if (recv_buf.empty()) {
        throw std::runtime_error("Memory allocation failed");
      }

      MPI_Recv(recv_buf.data(), count, datatype, i, 0, comm, MPI_STATUS_IGNORE);
      BaranovACustomAllreduceMPI::PerformOperation(recv_buf.data(), temp_buf.data(), count, datatype, op);
    }
  }
}

void ProcessRootSend(std::vector<unsigned char> &temp_buf, void *recvbuf, int count, MPI_Datatype datatype,
                     MPI_Comm comm, int root, int size, int type_size) {
  for (int i = 0; i < size; i++) {
    if (i != root) {
      MPI_Send(temp_buf.data(), count, datatype, i, 1, comm);
    }
  }
  std::memcpy(recvbuf, temp_buf.data(), static_cast<std::size_t>(count) * static_cast<std::size_t>(type_size));
}

}  // namespace

void BaranovACustomAllreduceMPI::CustomAllreduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
                                                 MPI_Op op, MPI_Comm comm, int root) {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &size);

  if (count == 0) {
    return;
  }

  int type_size = 0;
  MPI_Type_size(datatype, &type_size);

  std::vector<unsigned char> temp_buf(static_cast<std::size_t>(count) * static_cast<std::size_t>(type_size));
  if (temp_buf.empty()) {
    throw std::runtime_error("Memory allocation failed");
  }

  std::memcpy(temp_buf.data(), sendbuf, static_cast<std::size_t>(count) * static_cast<std::size_t>(type_size));

  if (rank == root) {
    ProcessRootReceive(temp_buf, count, datatype, op, comm, root, size, type_size);
    ProcessRootSend(temp_buf, recvbuf, count, datatype, comm, root, size, type_size);
  } else {
    MPI_Send(sendbuf, count, datatype, root, 0, comm);
    MPI_Recv(recvbuf, count, datatype, root, 1, comm, MPI_STATUS_IGNORE);
  }
}

template <typename T>
std::vector<T> BaranovACustomAllreduceMPI::GetVectorFromVariant(const InTypeVariant &variant) {
  try {
    return std::get<std::vector<T>>(variant);
  } catch (const std::bad_variant_access &) {
    throw std::runtime_error("Wrong variant type accessed");
  }
}

template std::vector<double> BaranovACustomAllreduceMPI::GetVectorFromVariant<double>(const InTypeVariant &variant);

BaranovACustomAllreduceMPI::BaranovACustomAllreduceMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  if (std::holds_alternative<std::vector<int>>(in)) {
    auto vec = std::get<std::vector<int>>(in);
    GetOutput() = InTypeVariant{std::vector<int>(vec.size(), 0)};
  } else if (std::holds_alternative<std::vector<float>>(in)) {
    auto vec = std::get<std::vector<float>>(in);
    GetOutput() = InTypeVariant{std::vector<float>(vec.size(), 0.0F)};
  } else {
    auto vec = std::get<std::vector<double>>(in);
    GetOutput() = InTypeVariant{std::vector<double>(vec.size(), 0.0)};
  }
}

bool BaranovACustomAllreduceMPI::ValidationImpl() {
  return true;
}

bool BaranovACustomAllreduceMPI::PreProcessingImpl() {
  return true;
}

bool BaranovACustomAllreduceMPI::RunImpl() {
  try {
    auto input = GetInput();
    auto output = GetOutput();

    if (std::holds_alternative<std::vector<int>>(input)) {
      auto data = std::get<std::vector<int>>(input);
      if (data.empty()) {
        GetOutput() = InTypeVariant{std::vector<int>{}};
        return true;
      }
      auto result_data = std::get<std::vector<int>>(output);
      CustomAllreduce(data.data(), result_data.data(), static_cast<int>(data.size()), MPI_INT, MPI_SUM, MPI_COMM_WORLD,
                      0);

      GetOutput() = InTypeVariant{result_data};
    } else if (std::holds_alternative<std::vector<float>>(input)) {
      auto data = std::get<std::vector<float>>(input);
      if (data.empty()) {
        GetOutput() = InTypeVariant{std::vector<float>{}};
        return true;
      }
      auto result_data = std::get<std::vector<float>>(output);
      CustomAllreduce(data.data(), result_data.data(), static_cast<int>(data.size()), MPI_FLOAT, MPI_SUM,
                      MPI_COMM_WORLD, 0);
      GetOutput() = InTypeVariant{result_data};
    } else if (std::holds_alternative<std::vector<double>>(input)) {
      auto data = std::get<std::vector<double>>(input);
      if (data.empty()) {
        GetOutput() = InTypeVariant{std::vector<double>{}};
        return true;
      }
      auto result_data = std::get<std::vector<double>>(output);
      CustomAllreduce(data.data(), result_data.data(), static_cast<int>(data.size()), MPI_DOUBLE, MPI_SUM,
                      MPI_COMM_WORLD, 0);
      GetOutput() = InTypeVariant{result_data};
    }
    return true;
  } catch (const std::exception &) {
    return false;
  }
}

bool BaranovACustomAllreduceMPI::PostProcessingImpl() {
  return true;
}

}  // namespace baranov_a_custom_allreduce
