#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace olesnitskiy_v_dijkstra_crs {

using InType = std::tuple<int, std::vector<int>, std::vector<int>, std::vector<int>>;
using OutType = std::vector<int>;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

struct GraphCRS {
  int vertices;
  int source;
  std::vector<int> offsets;
  std::vector<int> edges;
  std::vector<int> weights;
};

}  // namespace olesnitskiy_v_dijkstra_crs
