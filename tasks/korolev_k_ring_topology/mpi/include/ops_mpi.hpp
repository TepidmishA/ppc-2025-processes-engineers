#pragma once

#include "korolev_k_ring_topology/common/include/common.hpp"
#include "task/include/task.hpp"

namespace korolev_k_ring_topology {

class KorolevKRingTopologyMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit KorolevKRingTopologyMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace korolev_k_ring_topology
