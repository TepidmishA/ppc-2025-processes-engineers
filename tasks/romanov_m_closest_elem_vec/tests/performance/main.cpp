#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>

#include "romanov_m_closest_elem_vec/common/include/common.hpp"
#include "romanov_m_closest_elem_vec/mpi/include/ops_mpi.hpp"
#include "romanov_m_closest_elem_vec/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace romanov_m_closest_elem_vec {

class RomanovMClosestElemVecRunPerfTestProcesses : public ppc::util::BaseRunPerfTests<InType, OutType> {
  InType input_data_;

  void SetUp() override {
    const size_t vector_size = 20000000;
    input_data_.resize(vector_size);

    for (size_t i = 0; i < vector_size; ++i) {
      input_data_[i] = static_cast<int>((static_cast<int64_t>(i) * 1234567 + 7) % 100000);
    }
  }
  // test 2
  bool CheckTestOutputData(OutType &output_data) final {
    auto [first_idx, second_idx] = output_data;
    return first_idx >= 0 && second_idx == first_idx + 1 && static_cast<size_t>(second_idx) < input_data_.size();
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(RomanovMClosestElemVecRunPerfTestProcesses, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, RomanovMClosestElemVecMPI, RomanovMClosestElemVecSEQ>(
    PPC_SETTINGS_example_processes);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = RomanovMClosestElemVecRunPerfTestProcesses::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, RomanovMClosestElemVecRunPerfTestProcesses, kGtestValues, kPerfTestName);

}  // namespace romanov_m_closest_elem_vec
