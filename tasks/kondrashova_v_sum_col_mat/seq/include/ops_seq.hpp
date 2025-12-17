#pragma once

#include "kondrashova_v_sum_col_mat/common/include/common.hpp"
#include "task/include/task.hpp"

namespace kondrashova_v_sum_col_mat {

class KondrashovaVSumColMatSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit KondrashovaVSumColMatSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace kondrashova_v_sum_col_mat
