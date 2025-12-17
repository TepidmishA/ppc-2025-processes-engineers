#pragma once

#include <cstddef>
#include <vector>

#include "chernov_t_max_matrix_columns/common/include/common.hpp"
#include "task/include/task.hpp"

namespace chernov_t_max_matrix_columns {

class ChernovTMaxMatrixColumnsSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit ChernovTMaxMatrixColumnsSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  std::size_t rows_ = 0;
  std::size_t cols_ = 0;
  std::vector<int> input_matrix_;
  bool valid_ = false;
};

}  // namespace chernov_t_max_matrix_columns
