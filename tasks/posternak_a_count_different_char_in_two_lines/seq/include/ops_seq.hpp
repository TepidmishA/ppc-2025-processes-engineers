#pragma once

#include "posternak_a_count_different_char_in_two_lines/common/include/common.hpp"
#include "task/include/task.hpp"

namespace posternak_a_count_different_char_in_two_lines {

class PosternakACountDifferentCharInTwoLinesSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit PosternakACountDifferentCharInTwoLinesSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace posternak_a_count_different_char_in_two_lines
