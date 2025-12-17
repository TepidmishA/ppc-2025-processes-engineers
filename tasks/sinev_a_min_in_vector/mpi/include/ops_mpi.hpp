#pragma once

#include "sinev_a_min_in_vector/common/include/common.hpp"
#include "task/include/task.hpp"

namespace sinev_a_min_in_vector {

class SinevAMinInVectorMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit SinevAMinInVectorMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace sinev_a_min_in_vector
