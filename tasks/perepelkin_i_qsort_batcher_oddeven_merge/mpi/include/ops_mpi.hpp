#pragma once

#include <cstddef>
#include <vector>

#include "perepelkin_i_qsort_batcher_oddeven_merge/common/include/common.hpp"
#include "task/include/task.hpp"

namespace perepelkin_i_qsort_batcher_oddeven_merge {

class PerepelkinIQsortBatcherOddEvenMergeMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit PerepelkinIQsortBatcherOddEvenMergeMPI(const InType &in);

 private:
  int proc_rank_{};
  int proc_num_{};

  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  void BcastSizes(size_t &original_size, size_t &padded_size);
  void DistributeData(const size_t &padded_size, const std::vector<double> &padded_input, std::vector<int> &counts,
                      std::vector<int> &displs, std::vector<double> &local_data);
  void BuildComparators(std::vector<std::pair<int, int>> &comparators) const;
  void ProcessComparators(const std::vector<int> &counts, std::vector<double> &local_data,
                          const std::vector<std::pair<int, int>> &comparators);
  void MergeBlocks(const std::vector<double> &local_data, const std::vector<double> &peer_buffer,
                   std::vector<double> &temp, bool keep_lower) const;
  void BuildStageS(const std::vector<int> &procs_up, const std::vector<int> &procs_down,
                   std::vector<std::pair<int, int>> &comparators) const;
  void BuildStageB(const std::vector<int> &procs, std::vector<std::pair<int, int>> &comparators) const;
};

}  // namespace perepelkin_i_qsort_batcher_oddeven_merge
