#pragma once

#include <cstddef>
#include <vector>

#include "perepelkin_i_batcher_oddeven_mergesort/common/include/common.hpp"
#include "task/include/task.hpp"

namespace perepelkin_i_batcher_oddeven_mergesort {

class PerepelkinIBatcherOddEvenMergeSortMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit PerepelkinIBatcherOddEvenMergeSortMPI(const InType &in);

 private:
  int proc_rank_{};
  int proc_num_{};

  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  void BcastSizes(size_t &original_size, size_t &padded_size);
  size_t NextPowerOfTwo(const size_t &value);
  void OddEvenMerge(std::vector<double> &data, const size_t left, const size_t size, const size_t gap);
  void OddEvenMergeSort(std::vector<double> &data, const size_t left, const size_t size);
  void DistributeData(const size_t &padded_size, const std::vector<double> &padded_input, std::vector<int> &counts,
                      std::vector<int> &displs, std::vector<double> &local_data);
};

}  // namespace perepelkin_i_batcher_oddeven_mergesort
