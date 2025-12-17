#pragma once

#include "task/include/task.hpp"
#include "zhurin_i_matrix_sums/common/include/common.hpp"

namespace zhurin_i_matrix_sums {

class ZhurinIMatrixSumsMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit ZhurinIMatrixSumsMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace zhurin_i_matrix_sums
