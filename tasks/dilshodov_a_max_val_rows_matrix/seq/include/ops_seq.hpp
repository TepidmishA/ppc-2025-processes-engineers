#pragma once

#include "dilshodov_a_max_val_rows_matrix/common/include/common.hpp"
#include "task/include/task.hpp"

namespace dilshodov_a_max_val_rows_matrix {

class MaxValRowsMatrixTaskSequential : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit MaxValRowsMatrixTaskSequential(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace dilshodov_a_max_val_rows_matrix
