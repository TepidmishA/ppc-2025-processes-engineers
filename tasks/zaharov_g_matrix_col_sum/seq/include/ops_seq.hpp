#pragma once

#include "task/include/task.hpp"
#include "zaharov_g_matrix_col_sum/common/include/common.hpp"

namespace zaharov_g_matrix_col_sum {

class ZaharovGMatrixColSumSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit ZaharovGMatrixColSumSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace zaharov_g_matrix_col_sum
