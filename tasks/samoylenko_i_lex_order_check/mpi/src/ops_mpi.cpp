#include "samoylenko_i_lex_order_check/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>

#include "samoylenko_i_lex_order_check/common/include/common.hpp"

namespace samoylenko_i_lex_order_check {

namespace {

void ExchangeLengths(int rank, const std::pair<std::string, std::string> *input, size_t &len1, size_t &len2) {
  if (rank == 0) {
    len1 = input->first.size();
    len2 = input->second.size();
  }
  MPI_Bcast(&len1, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
  MPI_Bcast(&len2, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
}

void ScatterData(int rank, int size, const std::pair<std::string, std::string> *input, size_t min_len,
                 size_t substr_size, size_t substr_start, size_t substr_len, std::string &local_str1,
                 std::string &local_str2) {
  if (rank == 0) {
    const std::string &str1 = input->first;
    const std::string &str2 = input->second;

    if (substr_len > 0) {
      std::copy_n(str1.begin() + static_cast<std::ptrdiff_t>(substr_start), substr_len, local_str1.begin());
      std::copy_n(str2.begin() + static_cast<std::ptrdiff_t>(substr_start), substr_len, local_str2.begin());
    }

    for (int proc = 1; proc < size; ++proc) {
      size_t proc_start = proc * substr_size;
      if (proc_start < min_len) {
        size_t proc_len = std::min(substr_size, min_len - proc_start);
        MPI_Send(str1.data() + proc_start, static_cast<int>(proc_len), MPI_CHAR, proc, 0, MPI_COMM_WORLD);
        MPI_Send(str2.data() + proc_start, static_cast<int>(proc_len), MPI_CHAR, proc, 1, MPI_COMM_WORLD);
      }
    }
  } else {
    if (substr_len > 0) {
      MPI_Recv(local_str1.data(), static_cast<int>(substr_len), MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv(local_str2.data(), static_cast<int>(substr_len), MPI_CHAR, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
  }
}

unsigned int FindLocalDifference(const std::string &s1, const std::string &s2, size_t offset, size_t default_val) {
  for (size_t i = 0; i < s1.size(); ++i) {
    if (s1[i] != s2[i]) {
      return static_cast<unsigned int>(offset + i);
    }
  }
  return static_cast<unsigned int>(default_val);
}

}  // namespace

SamoylenkoILexOrderCheckMPI::SamoylenkoILexOrderCheckMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = false;
}

bool SamoylenkoILexOrderCheckMPI::ValidationImpl() {
  return true;
}

bool SamoylenkoILexOrderCheckMPI::PreProcessingImpl() {
  return true;
}

bool SamoylenkoILexOrderCheckMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  size_t len1 = 0;
  size_t len2 = 0;
  const auto *input_ptr = (rank == 0) ? &GetInput() : nullptr;

  ExchangeLengths(rank, input_ptr, len1, len2);

  size_t min_len = std::min(len1, len2);
  size_t substr_size = (min_len + size - 1) / size;
  size_t substr_start = rank * substr_size;
  size_t substr_len = 0;
  if (substr_start < min_len) {
    substr_len = std::min(substr_size, min_len - substr_start);
  }

  std::string local_str1(substr_len, '\0');
  std::string local_str2(substr_len, '\0');

  ScatterData(rank, size, input_ptr, min_len, substr_size, substr_start, substr_len, local_str1, local_str2);

  unsigned int local_diff = FindLocalDifference(local_str1, local_str2, substr_start, min_len + 1);
  unsigned int global_diff = 0;

  MPI_Allreduce(&local_diff, &global_diff, 1, MPI_UNSIGNED, MPI_MIN, MPI_COMM_WORLD);

  int output = 0;
  if (rank == 0) {
    if (global_diff == static_cast<unsigned int>(min_len + 1)) {
      output = (len1 <= len2) ? 1 : 0;
    } else {
      output = (GetInput().first[global_diff] < GetInput().second[global_diff]) ? 1 : 0;
    }
  }

  MPI_Bcast(&output, 1, MPI_INT, 0, MPI_COMM_WORLD);
  GetOutput() = (output != 0);

  return true;
}

bool SamoylenkoILexOrderCheckMPI::PostProcessingImpl() {
  return true;
}

}  // namespace samoylenko_i_lex_order_check
