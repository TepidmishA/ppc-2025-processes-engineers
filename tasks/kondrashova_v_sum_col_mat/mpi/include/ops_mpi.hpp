#pragma once

#include "kondrashova_v_sum_col_mat/common/include/common.hpp"
#include "task/include/task.hpp"

namespace kondrashova_v_sum_col_mat {

class KondrashovaVSumColMatMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }

  explicit KondrashovaVSumColMatMPI(const InType &in);

 private:
  int rows_ = 0;
  int cols_ = 0;

  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace kondrashova_v_sum_col_mat
