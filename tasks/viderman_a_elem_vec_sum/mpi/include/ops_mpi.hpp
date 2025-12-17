#pragma once

#include "task/include/task.hpp"
#include "viderman_a_elem_vec_sum/common/include/common.hpp"

namespace viderman_a_elem_vec_sum {

class VidermanAElemVecSumMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit VidermanAElemVecSumMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace viderman_a_elem_vec_sum
