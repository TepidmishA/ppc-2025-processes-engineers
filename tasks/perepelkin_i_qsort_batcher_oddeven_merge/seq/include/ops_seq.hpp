#pragma once

#include "perepelkin_i_qsort_batcher_oddeven_merge/common/include/common.hpp"
#include "task/include/task.hpp"

namespace perepelkin_i_qsort_batcher_oddeven_merge {

class PerepelkinIQsortBatcherOddEvenMergeSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit PerepelkinIQsortBatcherOddEvenMergeSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace perepelkin_i_qsort_batcher_oddeven_merge
