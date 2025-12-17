#pragma once

#include <vector>

#include "task/include/task.hpp"
#include "zaharov_g_matrix_col_sum/common/include/common.hpp"

namespace zaharov_g_matrix_col_sum {

class ZaharovGMatrixColSumMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit ZaharovGMatrixColSumMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
  OutType SumColValues(int start, int end);
  static std::vector<int> CalcInterval(int thread_amount, int rank, int column_amount);
};

}  // namespace zaharov_g_matrix_col_sum
