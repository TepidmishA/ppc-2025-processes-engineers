#pragma once

#include "badanov_a_max_vec_elem/common/include/common.hpp"
#include "task/include/task.hpp"

namespace badanov_a_max_vec_elem {

class BadanovAMaxVecElemSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit BadanovAMaxVecElemSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace badanov_a_max_vec_elem
