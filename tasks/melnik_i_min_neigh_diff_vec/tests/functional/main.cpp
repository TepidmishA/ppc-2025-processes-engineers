#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "melnik_i_min_neigh_diff_vec/common/include/common.hpp"
#include "melnik_i_min_neigh_diff_vec/mpi/include/ops_mpi.hpp"
#include "melnik_i_min_neigh_diff_vec/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace melnik_i_min_neigh_diff_vec {

class MelnikIMinNeighDiffVecRunFuncTestsProcesses : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    const auto &[vector, description] = test_param;
    return description;
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    input_data_ = std::get<0>(params);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    const auto &vector = input_data_;
    if (vector.size() < 2) {
      return std::get<0>(output_data) == -1 && std::get<1>(output_data) == -1;
    }

    std::uint64_t expected_idx = 0;
    int min_diff = std::abs(vector[1] - vector[0]);
    for (std::uint64_t i = 1; i < vector.size() - 1; i++) {
      int curr_diff = std::abs(vector[i + 1] - vector[i]);
      if (curr_diff < min_diff || (curr_diff == min_diff && i < expected_idx)) {
        min_diff = curr_diff;
        expected_idx = i;
      }
    }

    auto [first, second] = output_data;
    return std::cmp_equal(static_cast<std::uint64_t>(first), expected_idx) &&
           std::cmp_equal(static_cast<std::uint64_t>(second), expected_idx + 1);
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
};

namespace {

TEST_P(MelnikIMinNeighDiffVecRunFuncTestsProcesses, MinNeighDiffTest) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 20> kTestParam = {
    std::make_tuple(std::vector<int>{1, 2, 3, 4}, "basic_increasing_4"),
    std::make_tuple(std::vector<int>{0, 0, 0, 0}, "all_zero"),
    std::make_tuple(std::vector<int>{-4, -3, -2, -1}, "negative_increasing"),
    std::make_tuple(std::vector<int>{1, -1, 2, -2, 3, -3}, "alternating_signs"),
    std::make_tuple(std::vector<int>{10000, 10001, 30000, 30001}, "large_numbers_small_diffs"),
    std::make_tuple(std::vector<int>{7, 9}, "two_elements"),
    std::make_tuple(std::vector<int>(100, 5), "constant_100"),
    std::make_tuple(
        [] {
  std::vector<int> v(1000);
  for (int i = 0; i < 1000; i++) {
    v[i] = i % 100;
  }
  return v;
}(), "size_1000_varied"),
    std::make_tuple(
        [] {
  std::vector<int> v(50);
  for (int i = 0; i < 50; i++) {
    v[i] = (i * 3 % 50) - 25;
  }
  return v;
}(), "random_case_1"),
    std::make_tuple(
        [] {
  std::vector<int> v(60);
  for (int i = 0; i < 60; i++) {
    v[i] = ((i * 7) % 13) - 6;
  }
  return v;
}(), "random_case_2"),
    std::make_tuple(std::vector<int>{0, 1, 0, 1, 0, 1}, "half_zero"),
    std::make_tuple(
        [] {
  std::vector<int> v(100);
  for (int i = 0; i < 100; i++) {
    v[i] = i * 2;
  }
  v[99] = v[98] + 1;  // Min diff at end
  return v;
}(), "ascending_with_min_at_end"),
    std::make_tuple(std::vector<int>(5000, 3), "constant_5000"),
    std::make_tuple(std::vector<int>{1000000, 1000001, -1000000, -999999}, "extreme_values"),
    std::make_tuple(std::vector<int>{1, 2, 3, 4, 5}, "equal_diffs_should_pick_first"),
    std::make_tuple(std::vector<int>{10, 8, 6, 4, 2}, "decreasing_sequence"),
    std::make_tuple(std::vector<int>{5, 6, 10, 15, 20}, "min_diff_at_start"),
    std::make_tuple(std::vector<int>{10, 15, 18, 19, 25, 30}, "min_diff_at_middle"),
    std::make_tuple(std::vector<int>{1, 2, 4, 5, 7, 8}, "alternating_min_diffs"),
    std::make_tuple(std::vector<int>{0, 1, 3, 5, 7, 0}, "zeros_at_ends")};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<MelnikIMinNeighDiffVecMPI, InType>(kTestParam, PPC_SETTINGS_melnik_i_min_neigh_diff_vec),
    ppc::util::AddFuncTask<MelnikIMinNeighDiffVecSEQ, InType>(kTestParam, PPC_SETTINGS_melnik_i_min_neigh_diff_vec));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName =
    MelnikIMinNeighDiffVecRunFuncTestsProcesses::PrintFuncTestName<MelnikIMinNeighDiffVecRunFuncTestsProcesses>;

INSTANTIATE_TEST_SUITE_P(MinNeighDiffTests, MelnikIMinNeighDiffVecRunFuncTestsProcesses, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace melnik_i_min_neigh_diff_vec
