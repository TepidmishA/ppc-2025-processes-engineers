#pragma once

#include <vector>

#include "olesnitskiy_v_dijkstra_crs/common/include/common.hpp"
#include "task/include/task.hpp"

namespace olesnitskiy_v_dijkstra_crs {
class OlesnitskiyVDijkstraCrsSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit OlesnitskiyVDijkstraCrsSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
  std::vector<int> DijkstraCrs(const GraphCRS &graph);
  int FindMinDistance(const std::vector<int> &distances, const std::vector<bool> &visited);
};
}  // namespace olesnitskiy_v_dijkstra_crs
