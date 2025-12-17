#pragma once

#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace korolev_k_ring_topology {

struct RingMessage {
  int source = 0;
  int dest = 0;
  std::vector<int> data;
};

using InType = RingMessage;
using OutType = std::vector<int>;

using TestType = std::tuple<int, int, std::vector<int>>;

using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace korolev_k_ring_topology
