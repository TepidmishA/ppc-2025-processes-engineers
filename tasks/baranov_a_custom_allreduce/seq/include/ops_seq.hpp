#pragma once

#include "baranov_a_custom_allreduce/common/include/common.hpp"
#include "task/include/task.hpp"

namespace baranov_a_custom_allreduce {

class BaranovACustomAllreduceSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit BaranovACustomAllreduceSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};
}  // namespace baranov_a_custom_allreduce
