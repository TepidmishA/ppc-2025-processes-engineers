#pragma once

#include <vector>

#include "romanov_m_closest_elem_vec/common/include/common.hpp"
#include "task/include/task.hpp"

namespace romanov_m_closest_elem_vec {

struct Result {
  Result(int diff, int idx)
      : diff(diff), idx(idx) {

        };
  int diff;
  int idx;
};

void CalculateDistribution(int total_size, int comm_size, std::vector<int> &send_counts, std::vector<int> &displs);

void LocalFindMinDiff(const std::vector<int> &local_data, int local_sz, int global_offset, Result &local_res);

void PerformBoundaryCheck(int rank, int comm_size, int local_sz, int global_offset, const std::vector<int> &local_data,
                          Result &local_res);

void UpdateResult(Result &current_res, int new_diff, int new_idx);

class RomanovMClosestElemVecMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit RomanovMClosestElemVecMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace romanov_m_closest_elem_vec
