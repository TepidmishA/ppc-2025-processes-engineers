#include "olesnitskiy_v_dijkstra_crs/seq/include/ops_seq.hpp"

#include <functional>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

#include "olesnitskiy_v_dijkstra_crs/common/include/common.hpp"

namespace olesnitskiy_v_dijkstra_crs {

OlesnitskiyVDijkstraCrsSEQ::OlesnitskiyVDijkstraCrsSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::vector<int>();
}

bool OlesnitskiyVDijkstraCrsSEQ::ValidationImpl() {
  const auto &input = GetInput();
  int source = std::get<0>(input);
  const auto &offsets = std::get<1>(input);
  const auto &edges = std::get<2>(input);
  const auto &weights = std::get<3>(input);
  if (offsets.empty()) {
    return false;
  }
  int vertices = static_cast<int>(offsets.size()) - 1;
  if (vertices <= 0) {
    return false;
  }
  if (source < 0 || source >= vertices) {
    return false;
  }
  if (edges.size() != weights.size()) {
    return false;
  }
  return true;
}

bool OlesnitskiyVDijkstraCrsSEQ::PreProcessingImpl() {
  return true;
}

bool OlesnitskiyVDijkstraCrsSEQ::RunImpl() {
  const auto &input = GetInput();
  int source = std::get<0>(input);
  const auto &offsets = std::get<1>(input);
  const auto &edges = std::get<2>(input);
  const auto &weights = std::get<3>(input);
  int vertices = static_cast<int>(offsets.size()) - 1;

  std::vector<int> distances(vertices, std::numeric_limits<int>::max());
  distances[source] = 0;

  using DistVertex = std::pair<int, int>;
  std::priority_queue<DistVertex, std::vector<DistVertex>, std::greater<>> pq;
  pq.emplace(0, source);

  while (!pq.empty()) {
    auto [current_dist, u] = pq.top();
    pq.pop();

    if (current_dist > distances[u]) {
      continue;
    }

    int start = offsets[u];
    int end = offsets[u + 1];

    for (int i = start; i < end; ++i) {
      int v = edges[i];
      int weight = weights[i];
      int new_dist = current_dist + weight;

      if (new_dist < distances[v]) {
        distances[v] = new_dist;
        pq.emplace(new_dist, v);
      }
    }
  }

  GetOutput() = distances;
  return true;
}

bool OlesnitskiyVDijkstraCrsSEQ::PostProcessingImpl() {
  return true;
}
}  // namespace olesnitskiy_v_dijkstra_crs
