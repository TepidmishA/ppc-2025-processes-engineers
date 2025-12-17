#pragma once

#include <functional>
#include <queue>
#include <utility>
#include <vector>

#include "olesnitskiy_v_dijkstra_crs/common/include/common.hpp"
#include "task/include/task.hpp"

namespace olesnitskiy_v_dijkstra_crs {

class OlesnitskiyVDijkstraCrsMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit OlesnitskiyVDijkstraCrsMPI(const InType &in);

  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

 private:
  struct Update {
    int vertex{0};
    int distance{0};
  };

  struct DistVertexPair {
    int dist{0};
    int vertex{0};
  };

  struct GraphData {
    int vertices{0};
    int source{0};
    std::vector<int> offsets;
    std::vector<int> edges;
    std::vector<int> weights;
  };

  struct DijkstraContext {
    int start_idx{0};
    int end_idx{0};
    int local_vertices{0};
    int active{1};
    std::vector<int> counts;
    std::vector<int> displs;
    std::vector<int> local_distances;
    std::vector<bool> local_visited;
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<>> pq;
    std::vector<std::vector<Update>> send_bufs;
  };

  static bool IsVertexLocal(int vertex, int start_idx, int end_idx);
  static int FindOwner(int vertex, const std::vector<int> &displs, const std::vector<int> &counts, int size);
  static void ProcessLocalVertex(int vertex, int distance, const std::vector<int> &offsets,
                                 const std::vector<int> &edges, const std::vector<int> &weights, DijkstraContext &ctx,
                                 int rank, int size);
  static void ProcessReceivedData(const std::vector<int> &recv_data, int total_recv, DijkstraContext &ctx);
  static void PrepareSendData(const std::vector<std::vector<Update>> &send_bufs, std::vector<int> &send_data);
  static void CalculateDisplacements(const std::vector<int> &sizes, std::vector<int> &displs, int &total);
  static void PrepareByteArrays(const std::vector<int> &sizes, const std::vector<int> &displs,
                                std::vector<int> &counts_bytes, std::vector<int> &displs_bytes);
  static GraphData BroadcastGraphData(int rank, int /*size*/, const InType &input);
  static DistVertexPair FindGlobalBestVertex(const DistVertexPair &local_best);
  static bool ShouldStopAlgorithm(const DistVertexPair &global_best);
  static void ExchangeUpdates(DijkstraContext &ctx);
  static DijkstraContext InitializeLocalData(int vertices, int size, int rank, int source);
  static bool FindLocalBestVertex(DijkstraContext &ctx, DistVertexPair &local_best);
  static void ProcessGlobalVertex(const DistVertexPair &global_best, const GraphData &graph, DijkstraContext &ctx,
                                  int rank, int size);
  static bool PerformDijkstraIteration(const GraphData &graph, DijkstraContext &ctx, int rank, int size);
  static void RunDijkstraAlgorithm(const GraphData &graph, DijkstraContext &ctx, int rank, int size);
  void CollectResults(const GraphData &graph, const DijkstraContext &ctx, int rank, int size);
};
}  // namespace olesnitskiy_v_dijkstra_crs
