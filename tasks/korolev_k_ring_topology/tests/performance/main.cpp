#include <gtest/gtest.h>
#include <mpi.h>

#include <cstddef>
#include <vector>

#include "korolev_k_ring_topology/common/include/common.hpp"
#include "korolev_k_ring_topology/mpi/include/ops_mpi.hpp"
#include "korolev_k_ring_topology/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace korolev_k_ring_topology {

class KorolevKRingTopologyPerfTest : public ppc::util::BaseRunPerfTests<InType, OutType> {
  static constexpr int kDataSize = 1000000;
  InType input_data_{};

  void SetUp() override {
    int size = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    input_data_.source = 0;
    input_data_.dest = (size > 1) ? size - 1 : 0;
    input_data_.data.resize(kDataSize);
    for (int i = 0; i < kDataSize; ++i) {
      input_data_.data[static_cast<std::size_t>(i)] = i;
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return output_data == input_data_.data;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(KorolevKRingTopologyPerfTest, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, KorolevKRingTopologyMPI, KorolevKRingTopologySEQ>(
    PPC_SETTINGS_korolev_k_ring_topology);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = KorolevKRingTopologyPerfTest::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, KorolevKRingTopologyPerfTest, kGtestValues, kPerfTestName);

}  // namespace korolev_k_ring_topology
