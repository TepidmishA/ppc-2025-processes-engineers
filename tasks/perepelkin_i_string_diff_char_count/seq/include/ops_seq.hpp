#pragma once

#include "perepelkin_i_string_diff_char_count/common/include/common.hpp"
#include "task/include/task.hpp"

namespace perepelkin_i_string_diff_char_count {

class PerepelkinIStringDiffCharCountSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit PerepelkinIStringDiffCharCountSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace perepelkin_i_string_diff_char_count
