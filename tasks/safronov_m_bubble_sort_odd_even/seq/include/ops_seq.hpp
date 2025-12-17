#pragma once

#include "safronov_m_bubble_sort_odd_even/common/include/common.hpp"
#include "task/include/task.hpp"

namespace safronov_m_bubble_sort_odd_even {

class SafronovMBubbleSortOddEvenSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit SafronovMBubbleSortOddEvenSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace safronov_m_bubble_sort_odd_even
