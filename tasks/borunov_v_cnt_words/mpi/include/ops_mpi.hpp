#pragma once

#include <cstdint>
#include <vector>

#include "borunov_v_cnt_words/common/include/common.hpp"
#include "task/include/task.hpp"

namespace borunov_v_cnt_words {

class BorunovVCntWordsMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit BorunovVCntWordsMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
  static void CalculateDistribution(int text_len, int world_size, std::vector<int> &counts, std::vector<int> &displs);
  static uint64_t CountWordsLocal(const char *data, int count, char prev_char);
};

}  // namespace borunov_v_cnt_words
