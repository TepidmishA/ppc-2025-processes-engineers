#include "posternak_a_count_different_char_in_two_lines/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

#include "posternak_a_count_different_char_in_two_lines/common/include/common.hpp"

namespace posternak_a_count_different_char_in_two_lines {

PosternakACountDifferentCharInTwoLinesMPI::PosternakACountDifferentCharInTwoLinesMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool PosternakACountDifferentCharInTwoLinesMPI::ValidationImpl() {
  std::pair<std::string, std::string> &lines = GetInput();
  std::string s1 = lines.first;
  std::string s2 = lines.second;
  return !s1.empty() && !s2.empty();
}

bool PosternakACountDifferentCharInTwoLinesMPI::PreProcessingImpl() {
  return true;
}

bool PosternakACountDifferentCharInTwoLinesMPI::RunImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int size = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  std::string s1;
  std::string s2;

  int s1_len = 0;
  int s2_len = 0;

  if (rank == 0) {
    std::pair<std::string, std::string> &lines = GetInput();
    s1 = lines.first;
    s2 = lines.second;
    s1_len = static_cast<int>(s1.length());
    s2_len = static_cast<int>(s2.length());
  }

  MPI_Bcast(&s1_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&s2_len, 1, MPI_INT, 0, MPI_COMM_WORLD);

  std::vector<std::string> s1_proc_parts;
  std::vector<std::string> s2_proc_parts;

  if (rank == 0) {
    int min_len = std::min(s1_len, s2_len);

    int local_len = min_len / size;
    int remainder = min_len % size;

    int start = 0;
    for (int i = 0; i < size; i++) {
      int part_len = local_len;
      if (i == size - 1) {
        part_len += remainder;
      }

      s1_proc_parts.push_back(s1.substr(start, part_len));
      s2_proc_parts.push_back(s2.substr(start, part_len));
      start += part_len;
    }

    for (int proc = 1; proc < size; proc++) {
      int part_len = static_cast<int>(s1_proc_parts[proc].length());

      MPI_Send(&part_len, 1, MPI_INT, proc, 0, MPI_COMM_WORLD);
      MPI_Send(s1_proc_parts[proc].c_str(), part_len, MPI_CHAR, proc, 1, MPI_COMM_WORLD);
      MPI_Send(s2_proc_parts[proc].c_str(), part_len, MPI_CHAR, proc, 2, MPI_COMM_WORLD);
    }

    s1 = s1_proc_parts[0];
    s2 = s2_proc_parts[0];
  } else {
    int part_len = 0;
    MPI_Recv(&part_len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    s1.resize(part_len);
    s2.resize(part_len);

    MPI_Recv(s1.data(), part_len, MPI_CHAR, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(s2.data(), part_len, MPI_CHAR, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

  int process_count = 0;
  int part_len = static_cast<int>(s1.length());
  for (int i = 0; i < part_len; i++) {
    if (s1[i] != s2[i]) {
      process_count++;
    }
  }

  int count = 0;
  MPI_Allreduce(&process_count, &count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

  count += std::abs(s1_len - s2_len);
  GetOutput() = count;

  return true;
}

bool PosternakACountDifferentCharInTwoLinesMPI::PostProcessingImpl() {
  return true;
}

}  // namespace posternak_a_count_different_char_in_two_lines
