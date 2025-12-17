#include <gtest/gtest.h>

#include <cstddef>
#include <random>
#include <vector>

#include "melnik_i_min_neigh_diff_vec/common/include/common.hpp"
#include "melnik_i_min_neigh_diff_vec/mpi/include/ops_mpi.hpp"
#include "melnik_i_min_neigh_diff_vec/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace melnik_i_min_neigh_diff_vec {

class MelnikIMinNeighDiffVecRunPerfTestProcesses : public ppc::util::BaseRunPerfTests<InType, OutType> {
  InType input_data_;

  void GenerateVector(unsigned int seed) {
    const size_t vector_size = 200000000;

    std::vector<int> vector(vector_size);
    std::mt19937 generator(seed);
    std::uniform_int_distribution<int> distribution(-1000000, 1000000);

    for (size_t i = 0; i < vector_size; i++) {
      vector[i] = distribution(generator);
    }

    input_data_ = vector;
  }

  void SetUp() override {
    const unsigned int seed = 3301;
    GenerateVector(seed);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    auto [first, second] = output_data;
    return first >= 0 && second == first + 1 && static_cast<size_t>(second) < input_data_.size();
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(MelnikIMinNeighDiffVecRunPerfTestProcesses, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, MelnikIMinNeighDiffVecMPI, MelnikIMinNeighDiffVecSEQ>(
    PPC_SETTINGS_melnik_i_min_neigh_diff_vec);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = MelnikIMinNeighDiffVecRunPerfTestProcesses::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, MelnikIMinNeighDiffVecRunPerfTestProcesses, kGtestValues, kPerfTestName);

}  // namespace melnik_i_min_neigh_diff_vec
