#pragma once
#include <vector>

#include "remizov_k_max_in_matrix_string/common/include/common.hpp"
#include "task/include/task.hpp"

namespace remizov_k_max_in_matrix_string {

class RemizovKMaxInMatrixStringMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit RemizovKMaxInMatrixStringMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
  std::vector<int> FindMaxValues(int start, int end);
  static std::vector<int> CalculatingInterval(int size_prcs, int rank, int count_rows);
  void BroadcastResults(int rank);
};

}  // namespace remizov_k_max_in_matrix_string
