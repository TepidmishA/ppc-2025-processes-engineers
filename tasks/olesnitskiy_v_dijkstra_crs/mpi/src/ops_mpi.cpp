#include "olesnitskiy_v_dijkstra_crs/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <queue>
#include <tuple>
#include <vector>

#include "olesnitskiy_v_dijkstra_crs/common/include/common.hpp"

namespace olesnitskiy_v_dijkstra_crs {

OlesnitskiyVDijkstraCrsMPI::OlesnitskiyVDijkstraCrsMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::vector<int>();
}

bool OlesnitskiyVDijkstraCrsMPI::ValidationImpl() {
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

bool OlesnitskiyVDijkstraCrsMPI::PreProcessingImpl() {
  return true;
}

bool OlesnitskiyVDijkstraCrsMPI::IsVertexLocal(int vertex, int start_idx, int end_idx) {
  return vertex >= start_idx && vertex < end_idx;
}

int OlesnitskiyVDijkstraCrsMPI::FindOwner(int vertex, const std::vector<int> &displs, const std::vector<int> &counts,
                                          int size) {
  for (int j = 0; j < size; ++j) {
    if (vertex >= displs[j] && vertex < (displs[j] + counts[j])) {
      return j;
    }
  }
  return 0;
}

void OlesnitskiyVDijkstraCrsMPI::ProcessLocalVertex(int vertex, int distance, const std::vector<int> &offsets,
                                                    const std::vector<int> &edges, const std::vector<int> &weights,
                                                    DijkstraContext &ctx, int rank, int size) {
  int start = offsets[vertex];
  int end = offsets[vertex + 1];

  for (int i = start; i < end; ++i) {
    int neighbor = edges[i];
    int weight = weights[i];
    int new_dist = distance + weight;

    int owner = FindOwner(neighbor, ctx.displs, ctx.counts, size);

    if (owner == rank) {
      int neighbor_local_idx = neighbor - ctx.start_idx;
      if (!ctx.local_visited[neighbor_local_idx] && new_dist < ctx.local_distances[neighbor_local_idx]) {
        ctx.local_distances[neighbor_local_idx] = new_dist;
        ctx.pq.emplace(new_dist, neighbor);
      }
    } else {
      ctx.send_bufs[owner].push_back(Update{.vertex = neighbor, .distance = new_dist});
    }
  }
}

void OlesnitskiyVDijkstraCrsMPI::PrepareSendData(const std::vector<std::vector<Update>> &send_bufs,
                                                 std::vector<int> &send_data) {
  int idx = 0;
  for (const auto &send_buf : send_bufs) {
    for (const auto &update : send_buf) {
      send_data[idx++] = update.vertex;
      send_data[idx++] = update.distance;
    }
  }
}

void OlesnitskiyVDijkstraCrsMPI::ProcessReceivedData(const std::vector<int> &recv_data, int total_recv,
                                                     DijkstraContext &ctx) {
  for (int i = 0; i < total_recv * 2; i += 2) {
    int neighbor = recv_data[i];
    int new_dist = recv_data[i + 1];

    if (!IsVertexLocal(neighbor, ctx.start_idx, ctx.end_idx)) {
      continue;
    }

    int local_idx = neighbor - ctx.start_idx;
    bool should_update = !ctx.local_visited[local_idx] && new_dist < ctx.local_distances[local_idx];
    if (!should_update) {
      continue;
    }

    ctx.local_distances[local_idx] = new_dist;
    ctx.pq.emplace(new_dist, neighbor);
  }
}

void OlesnitskiyVDijkstraCrsMPI::CalculateDisplacements(const std::vector<int> &sizes, std::vector<int> &displs,
                                                        int &total) {
  total = 0;
  for (size_t i = 0; i < sizes.size(); ++i) {
    displs[i] = total;
    total += sizes[i];
  }
}

void OlesnitskiyVDijkstraCrsMPI::PrepareByteArrays(const std::vector<int> &sizes, const std::vector<int> &displs,
                                                   std::vector<int> &counts_bytes, std::vector<int> &displs_bytes) {
  for (size_t i = 0; i < sizes.size(); ++i) {
    counts_bytes[i] = sizes[i] * 2;
    displs_bytes[i] = displs[i] * 2;
  }
}

void OlesnitskiyVDijkstraCrsMPI::ExchangeUpdates(DijkstraContext &ctx) {
  int size = static_cast<int>(ctx.send_bufs.size());
  std::vector<int> send_sizes(size);
  std::vector<int> recv_sizes(size);

  for (int i = 0; i < size; ++i) {
    send_sizes[i] = static_cast<int>(ctx.send_bufs[i].size());
  }

  MPI_Alltoall(send_sizes.data(), 1, MPI_INT, recv_sizes.data(), 1, MPI_INT, MPI_COMM_WORLD);

  std::vector<int> send_displs(size);
  std::vector<int> recv_displs(size);
  int total_send = 0;
  int total_recv = 0;

  CalculateDisplacements(send_sizes, send_displs, total_send);
  CalculateDisplacements(recv_sizes, recv_displs, total_recv);

  const auto send_data_size = static_cast<std::size_t>(total_send) * 2U;
  const auto recv_data_size = static_cast<std::size_t>(total_recv) * 2U;
  std::vector<int> send_data(send_data_size);
  std::vector<int> recv_data(recv_data_size);

  PrepareSendData(ctx.send_bufs, send_data);

  for (int i = 0; i < size; ++i) {
    ctx.send_bufs[i].clear();
  }

  std::vector<int> send_counts_bytes(size);
  std::vector<int> recv_counts_bytes(size);
  std::vector<int> send_displs_bytes(size);
  std::vector<int> recv_displs_bytes(size);

  PrepareByteArrays(send_sizes, send_displs, send_counts_bytes, send_displs_bytes);
  PrepareByteArrays(recv_sizes, recv_displs, recv_counts_bytes, recv_displs_bytes);

  MPI_Alltoallv(send_data.data(), send_counts_bytes.data(), send_displs_bytes.data(), MPI_INT, recv_data.data(),
                recv_counts_bytes.data(), recv_displs_bytes.data(), MPI_INT, MPI_COMM_WORLD);

  ProcessReceivedData(recv_data, total_recv, ctx);
}

OlesnitskiyVDijkstraCrsMPI::DijkstraContext OlesnitskiyVDijkstraCrsMPI::InitializeLocalData(int vertices, int size,
                                                                                            int rank, int source) {
  DijkstraContext ctx;
  ctx.counts.resize(size);
  ctx.displs.resize(size);

  for (int idx = 0; idx < size; ++idx) {
    ctx.counts[idx] = (vertices / size) + (idx < (vertices % size) ? 1 : 0);
    ctx.displs[idx] = (idx == 0) ? 0 : ctx.displs[idx - 1] + ctx.counts[idx - 1];
  }

  ctx.start_idx = ctx.displs[rank];
  ctx.end_idx = ctx.start_idx + ctx.counts[rank];
  ctx.local_vertices = ctx.counts[rank];

  ctx.local_distances.resize(ctx.local_vertices, std::numeric_limits<int>::max());
  ctx.local_visited.resize(ctx.local_vertices, false);

  if (IsVertexLocal(source, ctx.start_idx, ctx.end_idx)) {
    ctx.local_distances[source - ctx.start_idx] = 0;
    ctx.pq.emplace(0, source);
  }

  ctx.send_bufs.resize(size);
  return ctx;
}

OlesnitskiyVDijkstraCrsMPI::GraphData OlesnitskiyVDijkstraCrsMPI::BroadcastGraphData(int rank, int /*size*/,
                                                                                     const InType &input) {
  GraphData graph;

  if (rank == 0) {
    const auto &inp = input;
    graph.source = std::get<0>(inp);
    graph.offsets = std::get<1>(inp);
    graph.edges = std::get<2>(inp);
    graph.weights = std::get<3>(inp);
    graph.vertices = static_cast<int>(graph.offsets.size()) - 1;
  }

  MPI_Bcast(&graph.vertices, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&graph.source, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    graph.offsets.resize(graph.vertices + 1);
  }
  MPI_Bcast(graph.offsets.data(), graph.vertices + 1, MPI_INT, 0, MPI_COMM_WORLD);

  int total_edges = 0;
  if (rank == 0) {
    total_edges = static_cast<int>(graph.edges.size());
  }
  MPI_Bcast(&total_edges, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    graph.edges.resize(total_edges);
    graph.weights.resize(total_edges);
  }
  MPI_Bcast(graph.edges.data(), total_edges, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(graph.weights.data(), total_edges, MPI_INT, 0, MPI_COMM_WORLD);

  return graph;
}

bool OlesnitskiyVDijkstraCrsMPI::FindLocalBestVertex(DijkstraContext &ctx, DistVertexPair &local_best) {
  local_best.dist = std::numeric_limits<int>::max();
  local_best.vertex = -1;

  if (!ctx.pq.empty()) {
    local_best.dist = ctx.pq.top().first;
    local_best.vertex = ctx.pq.top().second;
    int local_idx = local_best.vertex - ctx.start_idx;

    if (ctx.local_visited[local_idx]) {
      ctx.pq.pop();
      return false;
    }
  }

  return true;
}

OlesnitskiyVDijkstraCrsMPI::DistVertexPair OlesnitskiyVDijkstraCrsMPI::FindGlobalBestVertex(
    const DistVertexPair &local_best) {
  DistVertexPair global_best{};
  MPI_Allreduce(&local_best, &global_best, 1, MPI_2INT, MPI_MINLOC, MPI_COMM_WORLD);
  return global_best;
}

bool OlesnitskiyVDijkstraCrsMPI::ShouldStopAlgorithm(const DistVertexPair &global_best) {
  return global_best.vertex == -1 || global_best.dist == std::numeric_limits<int>::max();
}

void OlesnitskiyVDijkstraCrsMPI::ProcessGlobalVertex(const DistVertexPair &global_best, const GraphData &graph,
                                                     DijkstraContext &ctx, int rank, int size) {
  if (!IsVertexLocal(global_best.vertex, ctx.start_idx, ctx.end_idx)) {
    return;
  }

  int local_idx = global_best.vertex - ctx.start_idx;
  if (ctx.local_visited[local_idx]) {
    return;
  }

  ctx.local_visited[local_idx] = true;
  if (!ctx.pq.empty() && ctx.pq.top().second == global_best.vertex) {
    ctx.pq.pop();
  }

  ProcessLocalVertex(global_best.vertex, global_best.dist, graph.offsets, graph.edges, graph.weights, ctx, rank, size);
}

bool OlesnitskiyVDijkstraCrsMPI::PerformDijkstraIteration(const GraphData &graph, DijkstraContext &ctx, int rank,
                                                          int size) {
  DistVertexPair local_best;
  if (!FindLocalBestVertex(ctx, local_best)) {
    return true;
  }

  DistVertexPair global_best = FindGlobalBestVertex(local_best);

  if (ShouldStopAlgorithm(global_best)) {
    return false;
  }

  ProcessGlobalVertex(global_best, graph, ctx, rank, size);
  ExchangeUpdates(ctx);

  int local_active = !ctx.pq.empty() ? 1 : 0;
  MPI_Allreduce(&local_active, &ctx.active, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

  return true;
}

void OlesnitskiyVDijkstraCrsMPI::RunDijkstraAlgorithm(const GraphData &graph, DijkstraContext &ctx, int rank,
                                                      int size) {
  ctx.active = 1;
  while (ctx.active > 0) {
    if (!PerformDijkstraIteration(graph, ctx, rank, size)) {
      break;
    }
  }
}

void OlesnitskiyVDijkstraCrsMPI::CollectResults(const GraphData &graph, const DijkstraContext &ctx, int rank,
                                                int size) {
  if (rank == 0) {
    std::vector<int> global_distances(graph.vertices, std::numeric_limits<int>::max());
    std::ranges::copy(ctx.local_distances, global_distances.begin() + ctx.start_idx);

    for (int src = 1; src < size; ++src) {
      MPI_Recv(global_distances.data() + ctx.displs[src], ctx.counts[src], MPI_INT, src, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
    }

    GetOutput() = global_distances;
  } else {
    MPI_Send(ctx.local_distances.data(), ctx.local_vertices, MPI_INT, 0, 0, MPI_COMM_WORLD);
    GetOutput() = std::vector<int>();
  }
}

bool OlesnitskiyVDijkstraCrsMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  GraphData graph = BroadcastGraphData(rank, size, GetInput());

  DijkstraContext ctx = InitializeLocalData(graph.vertices, size, rank, graph.source);

  RunDijkstraAlgorithm(graph, ctx, rank, size);

  CollectResults(graph, ctx, rank, size);

  return true;
}

bool OlesnitskiyVDijkstraCrsMPI::PostProcessingImpl() {
  return true;
}
}  // namespace olesnitskiy_v_dijkstra_crs
