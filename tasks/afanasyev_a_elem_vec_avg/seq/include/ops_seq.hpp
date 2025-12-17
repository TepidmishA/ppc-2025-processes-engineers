#pragma once

#include "afanasyev_a_elem_vec_avg/common/include/common.hpp"
#include "task/include/task.hpp"

namespace afanasyev_a_elem_vec_avg {

class AfanasyevAElemVecAvgSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit AfanasyevAElemVecAvgSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace afanasyev_a_elem_vec_avg
