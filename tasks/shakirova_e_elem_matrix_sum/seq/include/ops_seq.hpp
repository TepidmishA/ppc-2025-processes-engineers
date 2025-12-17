#pragma once

#include "shakirova_e_elem_matrix_sum/common/include/common.hpp"
#include "task/include/task.hpp"

namespace shakirova_e_elem_matrix_sum {

class ShakirovaEElemMatrixSumSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit ShakirovaEElemMatrixSumSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace shakirova_e_elem_matrix_sum
