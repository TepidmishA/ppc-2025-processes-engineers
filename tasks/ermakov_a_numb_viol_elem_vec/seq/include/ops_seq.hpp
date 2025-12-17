#pragma once

#include "ermakov_a_numb_viol_elem_vec/common/include/common.hpp"
#include "task/include/task.hpp"

namespace ermakov_a_numb_viol_elem_vec {

class ErmakovANumbViolElemVecSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit ErmakovANumbViolElemVecSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace ermakov_a_numb_viol_elem_vec
