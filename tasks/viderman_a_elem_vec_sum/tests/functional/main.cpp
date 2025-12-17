#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <tuple>

#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"
#include "viderman_a_elem_vec_sum/common/include/common.hpp"
#include "viderman_a_elem_vec_sum/mpi/include/ops_mpi.hpp"
#include "viderman_a_elem_vec_sum/seq/include/ops_seq.hpp"

namespace viderman_a_elem_vec_sum {

class VidermanAElemVecSumFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    auto [filename, expected_sum] = test_param;
    return filename.substr(0, filename.find('.'));
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    std::string filename = std::get<0>(params);
    expected_sum_ = std::get<1>(params);

    std::string abs_path = ppc::util::GetAbsoluteTaskPath(PPC_ID_viderman_a_elem_vec_sum, filename);
    std::ifstream file(abs_path);

    if (!file.is_open()) {
      throw std::runtime_error("Failed to open file: " + abs_path);
    }

    double num{};
    while (file >> num) {
      input_data_.push_back(num);
    }
    file.close();
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return std::fabs(expected_sum_ - output_data) < std::numeric_limits<double>::epsilon() * 100;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  OutType expected_sum_ = 0.0;
};

namespace {

TEST_P(VidermanAElemVecSumFuncTests, VectorSumFuncTests) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 15> kTestParam = {

    std::make_tuple("test_vec_perf_small_viderman.txt", 5152.0),
    std::make_tuple("test_vec_perf_medium_viderman.txt", 5006992.0),
    std::make_tuple("test_vec_perf_large_viderman.txt", 498810788.0),
    std::make_tuple("test_vec_perf_huge_viderman.txt", 50005606393.0),

    std::make_tuple("test_vec_empty_viderman.txt", 0.0),
    std::make_tuple("test_vec_single_positive_viderman.txt", 42.0),
    std::make_tuple("test_vec_single_negative_viderman.txt", -15.0),
    std::make_tuple("test_vec_all_zeros_viderman.txt", 0.0),
    std::make_tuple("test_vec_small_positive_viderman.txt", 120.0),
    std::make_tuple("test_vec_small_negative_viderman.txt", -120.0),
    std::make_tuple("test_vec_mixed_signs_viderman.txt", 8.0),
    std::make_tuple("test_vec_large_numbers_viderman.txt", 120000000.0),
    std::make_tuple("test_vec_simple_fractions_viderman.txt", 61.5),
    std::make_tuple("test_vec_repeated_elements_viderman.txt", 140.0),
    std::make_tuple("test_vec_alternating_pattern_viderman.txt", 0.0)};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<VidermanAElemVecSumMPI, InType>(kTestParam, PPC_SETTINGS_viderman_a_elem_vec_sum),
    ppc::util::AddFuncTask<VidermanAElemVecSumSEQ, InType>(kTestParam, PPC_SETTINGS_viderman_a_elem_vec_sum));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kFuncTestName = VidermanAElemVecSumFuncTests::PrintFuncTestName<VidermanAElemVecSumFuncTests>;

INSTANTIATE_TEST_SUITE_P(VectorSumFuncTests, VidermanAElemVecSumFuncTests, kGtestValues, kFuncTestName);

}  // namespace

}  // namespace viderman_a_elem_vec_sum
