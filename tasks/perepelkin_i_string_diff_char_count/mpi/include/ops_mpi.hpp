#pragma once

#include <cstddef>
#include <vector>

#include "perepelkin_i_string_diff_char_count/common/include/common.hpp"
#include "task/include/task.hpp"

namespace perepelkin_i_string_diff_char_count {

class PerepelkinIStringDiffCharCountMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit PerepelkinIStringDiffCharCountMPI(const InType &in);

 private:
  int proc_rank_{};
  int proc_num_{};

  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  void DistributeData(size_t min_len, std::vector<char> &local_s1, std::vector<char> &local_s2);
};

}  // namespace perepelkin_i_string_diff_char_count
