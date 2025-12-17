#pragma once

#include "ermakov_a_numb_viol_elem_vec/common/include/common.hpp"
#include "task/include/task.hpp"

namespace ermakov_a_numb_viol_elem_vec {

class ErmakovANumbViolElemVecMPI : public BaseTask {
 public:
  static constexpr auto GetStaticTypeOfTask() -> ppc::task::TypeOfTask {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit ErmakovANumbViolElemVecMPI(const InType &in);

 private:
  auto ValidationImpl() -> bool override;
  auto PreProcessingImpl() -> bool override;
  auto RunImpl() -> bool override;
  auto PostProcessingImpl() -> bool override;
};

}  // namespace ermakov_a_numb_viol_elem_vec
