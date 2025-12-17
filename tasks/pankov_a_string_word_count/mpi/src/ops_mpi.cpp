#include "pankov_a_string_word_count/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>
#include <utility>

#include "pankov_a_string_word_count/common/include/common.hpp"

namespace pankov_a_string_word_count {

PankovAStringWordCountMPI::PankovAStringWordCountMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool PankovAStringWordCountMPI::ValidationImpl() {
  return GetOutput() == 0;
}

bool PankovAStringWordCountMPI::PreProcessingImpl() {
  GetOutput() = 0;
  return true;
}

namespace {

int CountWordsLocal(const std::string &s, std::size_t start, std::size_t end) {
  int count = 0;
  bool in_word = false;

  for (std::size_t i = start; i < end; ++i) {
    auto uc = static_cast<unsigned char>(s[i]);
    if (std::isspace(uc) == 0) {
      if (!in_word) {
        in_word = true;
        ++count;
      }
    } else {
      in_word = false;
    }
  }

  return count;
}

}  // namespace

bool PankovAStringWordCountMPI::RunImpl() {
  int rank = 0;
  int size = 1;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  std::string s;
  int n = 0;
  if (rank == 0) {
    s = GetInput();
    n = static_cast<int>(s.size());
  }
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (n == 0) {
    GetOutput() = 0;
    MPI_Barrier(MPI_COMM_WORLD);
    return true;
  }

  if (rank != 0) {
    s.resize(static_cast<std::size_t>(n));
  }
  MPI_Bcast(s.data(), n, MPI_CHAR, 0, MPI_COMM_WORLD);

  const auto str_size = static_cast<std::size_t>(n);
  std::size_t base = str_size / static_cast<std::size_t>(size);
  std::size_t rem = str_size % static_cast<std::size_t>(size);

  std::size_t start = (rank * base) + static_cast<std::size_t>(std::min(rank, static_cast<int>(rem)));
  std::size_t end = start + base + (std::cmp_less(rank, static_cast<int>(rem)) ? 1 : 0);

  start = std::min(start, str_size);
  end = std::min(end, str_size);

  if (rank != 0 && start < str_size) {
    if ((std::isspace(static_cast<unsigned char>(s[start])) == 0) &&
        (std::isspace(static_cast<unsigned char>(s[start - 1])) == 0)) {
      while (start < end && (std::isspace(static_cast<unsigned char>(s[start])) == 0)) {
        ++start;
      }
    }
  }

  start = std::min(start, end);

  int local_count = 0;
  if (start < end) {
    local_count = CountWordsLocal(s, start, end);
  }

  int global_count = 0;

  MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  MPI_Bcast(&global_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

  GetOutput() = global_count;

  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

bool PankovAStringWordCountMPI::PostProcessingImpl() {
  return GetOutput() >= 0;
}

}  // namespace pankov_a_string_word_count
