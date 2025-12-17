#include "belov_e_lexico_order_two_strings/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <limits>
#include <string>
#include <vector>

#include "belov_e_lexico_order_two_strings/common/include/common.hpp"

namespace belov_e_lexico_order_two_strings {
BelovELexicoOrderTwoStringsMPI::BelovELexicoOrderTwoStringsMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = false;
}

bool BelovELexicoOrderTwoStringsMPI::ValidationImpl() {
  return !std::get<0>(GetInput()).empty() && !std::get<1>(GetInput()).empty();
}

bool BelovELexicoOrderTwoStringsMPI::PreProcessingImpl() {
  std::vector<std::string> temp;
  std::string current;
  for (auto &ch : std::get<0>(GetInput())) {
    if (ch == ' ') {
      temp.push_back(current);
      current = "";
    } else {
      current += ch;
    }
  }
  if (!current.empty()) {
    temp.push_back(current);
  }
  std::get<0>(GetProccesedInput()) = temp;

  std::vector<std::string>().swap(temp);
  current = "";
  for (auto &ch : std::get<1>(GetInput())) {
    if (ch == ' ') {
      temp.push_back(current);
      current = "";
    } else {
      current += ch;
    }
  }
  if (!current.empty()) {
    temp.push_back(current);
  }
  std::get<1>(GetProccesedInput()) = temp;

  return !std::get<0>(GetProccesedInput()).empty() && !std::get<1>(GetProccesedInput()).empty();
}

ChunkAns ChunkCheck(const std::vector<std::string> &first, const std::vector<std::string> &second, int begin, int end) {
  ChunkAns ans{.index = -1, .cmp_flag = 0};
  for (int i = begin; i < end; i++) {
    if (first[i] < second[i]) {
      ans = {.index = i, .cmp_flag = -1};
      return ans;
    }
    if (first[i] > second[i]) {
      ans = {.index = i, .cmp_flag = 1};
      return ans;
    }
  }
  return ans;
}

int CeilDiv(int a, int b) {
  return (a + b - 1) / b;
}

void BcastVectorOfStrings(std::vector<std::string> &vec, int n, MPI_Comm comm) {
  int rank = 0;
  MPI_Comm_rank(comm, &rank);

  if (rank != 0) {
    vec.resize(n);
  }

  for (int i = 0; i < n; i++) {
    int len = 0;
    if (rank == 0) {
      len = static_cast<int>(vec[i].size());
    }

    MPI_Bcast(&len, 1, MPI_INT, 0, comm);

    if (rank != 0) {
      vec[i].resize(len);
    }

    MPI_Bcast(vec[i].data(), len, MPI_CHAR, 0, comm);
  }
}

bool BelovELexicoOrderTwoStringsMPI::RunImpl() {
  int mpi_size = 0;
  int rank = 0;

  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  std::vector<std::string> first;
  std::vector<std::string> second;

  int n1 = 0;
  int n2 = 0;
  int n = 0;

  if (rank == 0) {
    first = std::get<0>(GetProccesedInput());
    second = std::get<1>(GetProccesedInput());

    n1 = static_cast<int>(first.size());
    n2 = static_cast<int>(second.size());
  }

  MPI_Bcast(&n1, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&n2, 1, MPI_INT, 0, MPI_COMM_WORLD);

  n = std::min(n1, n2);

  BcastVectorOfStrings(first, n, MPI_COMM_WORLD);
  BcastVectorOfStrings(second, n, MPI_COMM_WORLD);

  int chunk = CeilDiv(n, mpi_size);
  int begin = rank * chunk;
  int end = std::min(n, begin + chunk);

  ChunkAns local_ans = {.index = -1, .cmp_flag = 0};

  local_ans = ChunkCheck(first, second, begin, end);

  std::vector<ChunkAns> results(mpi_size);
  MPI_Gather(&local_ans, sizeof(ChunkAns), MPI_BYTE, results.data(), sizeof(ChunkAns), MPI_BYTE, 0, MPI_COMM_WORLD);

  bool result = false;

  if (rank == 0) {
    int best_index = std::numeric_limits<int>::max();
    int cmp_ans = 0;

    for (auto &p : results) {
      if (p.index >= 0 && p.index < best_index) {
        best_index = p.index;
        cmp_ans = p.cmp_flag;
      }
    }
    if (best_index != std::numeric_limits<int>::max()) {
      result = (cmp_ans < 0);
    } else {
      result = (n1 < n2);
    }
  }

  MPI_Bcast(&result, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);

  GetOutput() = result;

  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

bool BelovELexicoOrderTwoStringsMPI::PostProcessingImpl() {
  return true;
}
}  // namespace belov_e_lexico_order_two_strings
