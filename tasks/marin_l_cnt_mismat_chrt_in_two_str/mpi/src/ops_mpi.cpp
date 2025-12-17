#include "marin_l_cnt_mismat_chrt_in_two_str/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
#include <utility>

#include "marin_l_cnt_mismat_chrt_in_two_str/common/include/common.hpp"

namespace marin_l_cnt_mismat_chrt_in_two_str {

MarinLCntMismatChrtInTwoStrMPI::MarinLCntMismatChrtInTwoStrMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool MarinLCntMismatChrtInTwoStrMPI::ValidationImpl() {
  return true;
}

bool MarinLCntMismatChrtInTwoStrMPI::PreProcessingImpl() {
  int rank = 0;
  int size = 1;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  std::array<int, 2> lengths{};
  std::string s1_local;
  std::string s2_local;

  if (rank == 0) {
    s1_local = GetInput().first;
    s2_local = GetInput().second;
    lengths[0] = static_cast<int>(s1_local.size());
    lengths[1] = static_cast<int>(s2_local.size());
  }

  MPI_Bcast(lengths.data(), 2, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    s1_local.resize(lengths[0]);
    s2_local.resize(lengths[1]);
  }

  MPI_Bcast(s1_local.data(), lengths[0], MPI_CHAR, 0, MPI_COMM_WORLD);
  MPI_Bcast(s2_local.data(), lengths[1], MPI_CHAR, 0, MPI_COMM_WORLD);

  local_s1_ = std::move(s1_local);
  local_s2_ = std::move(s2_local);

  GetOutput() = 0;
  return true;
}

bool MarinLCntMismatChrtInTwoStrMPI::RunImpl() {
  int rank = 0;
  int size = 1;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  const std::string &s1 = local_s1_;
  const std::string &s2 = local_s2_;

  std::size_t len1 = s1.size();
  std::size_t len2 = s2.size();
  std::size_t total_len = std::max(len1, len2);

  if (total_len == 0) {
    GetOutput() = 0;
    return true;
  }

  std::size_t chunk = total_len / static_cast<std::size_t>(size);
  std::size_t remainder = total_len % static_cast<std::size_t>(size);

  std::size_t start =
      (static_cast<std::size_t>(rank) * chunk) + std::min<std::size_t>(static_cast<std::size_t>(rank), remainder);
  std::size_t end = start + chunk;
  if (std::cmp_less(rank, remainder)) {
    end += 1;
  }

  int local_count = 0;
  for (std::size_t i = start; i < end && i < total_len; ++i) {
    if (i >= len1 || i >= len2 || s1[i] != s2[i]) {
      ++local_count;
    }
  }

  int global_count = 0;
  MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Bcast(&global_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

  GetOutput() = global_count;
  return true;
}

bool MarinLCntMismatChrtInTwoStrMPI::PostProcessingImpl() {
  return true;
}

}  // namespace marin_l_cnt_mismat_chrt_in_two_str
