#pragma once

#include "ovchinnikov_m_bubble_sort/common/include/common.hpp"
#include "task/include/task.hpp"

namespace ovchinnikov_m_bubble_sort {

class OvchinnikovMBubbleSortSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit OvchinnikovMBubbleSortSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace ovchinnikov_m_bubble_sort
