#pragma once

#include "bortsova_a_max_elem_vector/common/include/common.hpp"
#include "task/include/task.hpp"

namespace bortsova_a_max_elem_vector {

class BortsovaAMaxElemVectorSeq : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit BortsovaAMaxElemVectorSeq(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace bortsova_a_max_elem_vector
