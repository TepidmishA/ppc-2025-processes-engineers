#pragma once

#include <cstddef>

#include "batkov_f_vector_sum/common/include/common.hpp"
#include "task/include/task.hpp"

namespace batkov_f_vector_sum {

class BatkovFVectorSumMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit BatkovFVectorSumMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  size_t m_rank_{0};
  size_t m_mpi_size_{0};
};

}  // namespace batkov_f_vector_sum
