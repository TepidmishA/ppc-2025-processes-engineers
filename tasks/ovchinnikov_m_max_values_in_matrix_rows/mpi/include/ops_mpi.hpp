#pragma once

#include "ovchinnikov_m_max_values_in_matrix_rows/common/include/common.hpp"
#include "task/include/task.hpp"

namespace ovchinnikov_m_max_values_in_matrix_rows {

class OvchinnikovMMaxValuesInMatrixRowsMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit OvchinnikovMMaxValuesInMatrixRowsMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace ovchinnikov_m_max_values_in_matrix_rows
