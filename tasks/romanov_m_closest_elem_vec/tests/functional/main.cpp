#include <gtest/gtest.h>
#include <stb/stb_image.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <string>
#include <tuple>
#include <vector>

#include "romanov_m_closest_elem_vec/common/include/common.hpp"
#include "romanov_m_closest_elem_vec/mpi/include/ops_mpi.hpp"
#include "romanov_m_closest_elem_vec/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace romanov_m_closest_elem_vec {

class RomanovMClosestElemVecRunFuncTestsProcesses : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    const auto &[vector, description] = test_param;
    return description;
  }

 protected:
  void SetUp() override {
    const auto &params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    input_data_ = std::get<0>(params);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    const auto &vector = input_data_;
    if (vector.size() < 2) {
      return std::get<0>(output_data) == -1 && std::get<1>(output_data) == -1;
    }

    int min_diff = std::abs(vector[1] - vector[0]);
    int expected_idx = 0;

    for (size_t i = 1; i < vector.size() - 1; ++i) {
      int curr_diff = std::abs(vector[i + 1] - vector[i]);
      if (curr_diff < min_diff) {
        min_diff = curr_diff;
        expected_idx = static_cast<int>(i);
      }
    }

    auto [res_idx_1, res_idx_2] = output_data;

    if (res_idx_2 != res_idx_1 + 1) {
      return false;
    }

    int found_diff = std::abs(vector[res_idx_2] - vector[res_idx_1]);

    return res_idx_1 == expected_idx && found_diff == min_diff;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
};

namespace {

TEST_P(RomanovMClosestElemVecRunFuncTestsProcesses, MinNeighDiffTest) {
  ExecuteTest(GetParam());
}

std::vector<int> GenerateSawtooth(int size, int period, int amplitude) {
  std::vector<int> v(size);
  for (int i = 0; i < size; ++i) {
    v[i] = std::abs((i % period) - amplitude);
  }
  return v;
}

const std::array<TestType, 14> kTestCases = {{
    {{10, 20, 30, 30, 40, 50}, "zero_diff_middle"},
    {{1, 3, 5, 7, 8, 10, 12}, "min_diff_boundary_check"},
    {{100, 90, 80, 75, 70, 60}, "decreasing_sequence"},
    {{0, 1000, 2000, 2001, 3000, 4000}, "large_jumps_small_step"},
    {{0, 10, 0, 10, 0, 10}, "zig_zag_pattern"},
    {{1, 1, 2, 3, 5, 8, 13, 21, 34}, "fibonacci_growing_gaps"},
    {{-10, -20, -25, -40, -50}, "negative_numbers"},
    {{-5, -1, 1, 10, 20}, "crossing_zero_min_diff"},
    {GenerateSawtooth(100, 20, 10), "sawtooth_wave_100"},
    {std::vector<int>(50, 777), "all_identical"},
    {{42, 100}, "minimal_size_2"},
    {{15, 92, 11, 4, 1005, 1006, 33, 99}, "random_noise_manual_min"},
    {[]() {
  std::vector<int> v(200);
  for (int i = 0; i < 200; i++) {
    v[i] = i * 3;
  }
  v[150] = v[149] + 1;
  return v;
}(), "long_monotonic_break"},
    {{0, 100, 101, 200, 201, 300}, "chess_diffs"},
}};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<RomanovMClosestElemVecMPI, InType>(kTestCases, PPC_SETTINGS_example_processes),
    ppc::util::AddFuncTask<RomanovMClosestElemVecSEQ, InType>(kTestCases, PPC_SETTINGS_example_processes));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName =
    RomanovMClosestElemVecRunFuncTestsProcesses::PrintFuncTestName<RomanovMClosestElemVecRunFuncTestsProcesses>;

INSTANTIATE_TEST_SUITE_P(MinNeighDiffTests, RomanovMClosestElemVecRunFuncTestsProcesses, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace romanov_m_closest_elem_vec
