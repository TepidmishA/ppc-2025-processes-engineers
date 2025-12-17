#include "borunov_v_cnt_words/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cctype>
#include <cstdint>
#include <string>
#include <vector>

#include "borunov_v_cnt_words/common/include/common.hpp"

namespace borunov_v_cnt_words {

BorunovVCntWordsMPI::BorunovVCntWordsMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool BorunovVCntWordsMPI::ValidationImpl() {
  return true;
}

bool BorunovVCntWordsMPI::PreProcessingImpl() {
  return true;
}

void BorunovVCntWordsMPI::CalculateDistribution(int text_len, int world_size, std::vector<int> &counts,
                                                std::vector<int> &displs) {
  counts.resize(world_size);
  displs.resize(world_size);

  if (world_size == 0) {
    return;
  }

  int base_count = text_len / world_size;
  int remainder = text_len % world_size;
  int current_displ = 0;

  for (int i = 0; i < world_size; ++i) {
    counts[i] = base_count + (i < remainder ? 1 : 0);
    displs[i] = current_displ;
    current_displ += counts[i];
  }
}

uint64_t BorunovVCntWordsMPI::CountWordsLocal(const char *data, int count, char prev_char) {
  if (count == 0) {
    return 0;
  }

  uint64_t local_cnt = 0;

  for (int i = 0; i < count; ++i) {
    auto current = static_cast<unsigned char>(data[i]);

    if (std::isspace(current) == 0) {
      unsigned char prev = 0;

      if (i == 0) {
        prev = static_cast<unsigned char>(prev_char);
      } else {
        prev = static_cast<unsigned char>(data[i - 1]);
      }

      if (std::isspace(prev) != 0) {
        local_cnt++;
      }
    }
  }
  return local_cnt;
}

bool BorunovVCntWordsMPI::RunImpl() {
  int rank = 0;
  int world_size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  int text_len = 0;
  if (rank == 0) {
    text_len = static_cast<int>(GetInput().size());
  }

  MPI_Bcast(&text_len, 1, MPI_INT, 0, MPI_COMM_WORLD);

  uint64_t global_result = 0;

  if (text_len < world_size) {
    if (rank == 0) {
      global_result = CountWordsLocal(GetInput().data(), text_len, ' ');
    }
    MPI_Bcast(&global_result, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
    GetOutput() = global_result;
    return true;
  }

  std::vector<int> send_counts;
  std::vector<int> displs;
  CalculateDistribution(text_len, world_size, send_counts, displs);

  int local_count = send_counts[rank];
  std::vector<char> local_data(local_count);

  const char *send_buf = (rank == 0) ? GetInput().data() : nullptr;

  MPI_Scatterv(send_buf, send_counts.data(), displs.data(), MPI_CHAR, local_data.data(), local_count, MPI_CHAR, 0,
               MPI_COMM_WORLD);

  int left_neighbor = (rank == 0) ? MPI_PROC_NULL : rank - 1;
  int right_neighbor = (rank == world_size - 1) ? MPI_PROC_NULL : rank + 1;

  char char_to_send = ' ';
  if (local_count > 0) {
    char_to_send = local_data.back();
  }

  char prev_char = ' ';
  MPI_Sendrecv(&char_to_send, 1, MPI_CHAR, right_neighbor, 0, &prev_char, 1, MPI_CHAR, left_neighbor, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);

  uint64_t local_result = 0;
  if (local_count > 0) {
    local_result = CountWordsLocal(local_data.data(), local_count, prev_char);
  }

  MPI_Reduce(&local_result, &global_result, 1, MPI_UINT64_T, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Bcast(&global_result, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);

  GetOutput() = global_result;

  return true;
}

bool BorunovVCntWordsMPI::PostProcessingImpl() {
  return true;
}

}  // namespace borunov_v_cnt_words
