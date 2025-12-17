#pragma once

#include "batkov_f_vector_sum/common/include/common.hpp"
#include "task/include/task.hpp"

namespace batkov_f_vector_sum {

class BatkovFVectorSumSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit BatkovFVectorSumSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace batkov_f_vector_sum
