#pragma once

#include <vector>

#include "dilshodov_a_max_val_rows_matrix/common/include/common.hpp"
#include "task/include/task.hpp"

namespace dilshodov_a_max_val_rows_matrix {

class MaxValRowsMatrixTaskMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit MaxValRowsMatrixTaskMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  void RunMaster(int rows, int cols, int base_rows, int extra_rows, int local_rows, std::vector<int> &local_matrix,
                 std::vector<int> &local_max);
  static void RunWorker(int cols, int local_rows, std::vector<int> &local_matrix, std::vector<int> &local_max);
};

}  // namespace dilshodov_a_max_val_rows_matrix
