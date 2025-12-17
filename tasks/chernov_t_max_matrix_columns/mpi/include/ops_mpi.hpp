#pragma once

#include <cstddef>
#include <vector>

#include "chernov_t_max_matrix_columns/common/include/common.hpp"
#include "task/include/task.hpp"

namespace chernov_t_max_matrix_columns {

class ChernovTMaxMatrixColumnsMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit ChernovTMaxMatrixColumnsMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  void BroadcastDimensions(int rank);
  std::vector<int> ScatterMatrixData(int rank, int size);
  [[nodiscard]] std::vector<int> ComputeLocalMaxima(int rank, int size, const std::vector<int> &local_data) const;
  void ComputeAndBroadcastResult(const std::vector<int> &local_maxima);

  std::size_t rows_ = 0;
  std::size_t cols_ = 0;
  std::vector<int> input_matrix_;
  bool valid_ = false;
  int total_rows_ = 0;
  int total_cols_ = 0;
};

}  // namespace chernov_t_max_matrix_columns
