#pragma once

#include "perepelkin_i_batcher_oddeven_mergesort/common/include/common.hpp"
#include "task/include/task.hpp"

namespace perepelkin_i_batcher_oddeven_mergesort {

class PerepelkinIBatcherOddEvenMergeSortSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit PerepelkinIBatcherOddEvenMergeSortSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  size_t NextPowerOfTwo(const size_t &value);
  void OddEvenMergeSort(std::vector<double> &data, const size_t left, const size_t size);
  void OddEvenMerge(std::vector<double> &data, const size_t left, const size_t size, const size_t gap);
};

}  // namespace perepelkin_i_batcher_oddeven_mergesort
