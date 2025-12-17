#include <gtest/gtest.h>

#include <limits>
#include <tuple>
#include <vector>

#include "olesnitskiy_v_dijkstra_crs/common/include/common.hpp"
#include "olesnitskiy_v_dijkstra_crs/mpi/include/ops_mpi.hpp"
#include "olesnitskiy_v_dijkstra_crs/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace olesnitskiy_v_dijkstra_crs {
class OlesnitskiyVDijkstraCrsPerfTest : public ppc::util::BaseRunPerfTests<InType, OutType> {
  const int kVertices_ = 1000000;
  [[nodiscard]] InType GenerateTestGraph() const {
    std::vector<int> offsets(kVertices_ + 1, 0);
    std::vector<int> edges;
    std::vector<int> weights;
    for (int i = 0; i < kVertices_; ++i) {
      for (int edge_num = 0; edge_num < 100; ++edge_num) {
        int j = (i + edge_num * 97) % kVertices_;
        if (j == i) {
          j = (j + 1) % kVertices_;
        }
        edges.push_back(j);
        int weight = 1 + ((i + j + edge_num) % 20);
        weights.push_back(weight);
        offsets[i + 1]++;
      }
    }
    for (int i = 0; i < kVertices_; ++i) {
      offsets[i + 1] += offsets[i];
    }
    int source = 0;
    return std::make_tuple(source, offsets, edges, weights);
  }

  InType input_data_;

  void SetUp() override {
    input_data_ = GenerateTestGraph();
  }

  bool CheckTestOutputData(OutType &output_data) final {
    if (output_data.empty()) {
      return true;
    }
    if (!output_data.empty()) {
      if (output_data[0] != 0) {
        return false;
      }
      for (int dist : output_data) {
        if (dist < 0) {
          return false;
        }
      }
      int reachable_count = 0;
      for (int dist : output_data) {
        if (dist < std::numeric_limits<int>::max()) {
          reachable_count++;
        }
      }
      if (reachable_count < kVertices_ / 2) {
        return false;
      }
    }

    return true;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(OlesnitskiyVDijkstraCrsPerfTest, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, OlesnitskiyVDijkstraCrsMPI, OlesnitskiyVDijkstraCrsSEQ>(
    PPC_SETTINGS_olesnitskiy_v_dijkstra_crs);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = OlesnitskiyVDijkstraCrsPerfTest::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, OlesnitskiyVDijkstraCrsPerfTest, kGtestValues, kPerfTestName);

}  // namespace olesnitskiy_v_dijkstra_crs
