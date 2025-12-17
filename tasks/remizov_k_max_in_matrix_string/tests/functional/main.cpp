#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <vector>

#include "remizov_k_max_in_matrix_string/common/include/common.hpp"
#include "remizov_k_max_in_matrix_string/mpi/include/ops_mpi.hpp"
#include "remizov_k_max_in_matrix_string/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace remizov_k_max_in_matrix_string {

class RemizovKRunFuncMaxInMatrixString : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    const auto &input_matrix = std::get<0>(test_param);
    const auto &expected_output = std::get<1>(test_param);

    std::string test_name = "matrix_" + std::to_string(input_matrix.size()) + "x" +
                            (input_matrix.empty() ? "0" : std::to_string(input_matrix[0].size()));
    if (!expected_output.empty()) {
      test_name += "_maxes";
      for (size_t i = 0; i < expected_output.size(); ++i) {
        test_name += "_" + FormatNumber(expected_output[i]);
      }
    }

    return test_name;
  }

 private:
  static std::string FormatNumber(int num) {
    if (num >= 0) {
      return std::to_string(num);
    }
    return "neg" + std::to_string(-num);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    input_data_ = std::get<0>(params);
    expected_output_ = std::get<1>(params);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    if (output_data.size() != expected_output_.size()) {
      return false;
    }

    for (size_t i = 0; i < output_data.size(); ++i) {
      if (output_data[i] != expected_output_[i]) {
        return false;
      }
    }

    return true;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  OutType expected_output_;
};

namespace {

TEST_P(RemizovKRunFuncMaxInMatrixString, FindMaxInEachRow) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 6> kTestParam = {
    std::make_tuple(std::vector<std::vector<int>>{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}, std::vector<int>{3, 6, 9}),

    std::make_tuple(std::vector<std::vector<int>>{{-1, -5, -3}, {-9, -2, -7}}, std::vector<int>{-1, -2}),

    std::make_tuple(std::vector<std::vector<int>>{{5, 8, 2, 10, 1}}, std::vector<int>{10}),

    std::make_tuple(std::vector<std::vector<int>>{{7, 7, 7}, {7, 7, 7}}, std::vector<int>{7, 7}),

    std::make_tuple(std::vector<std::vector<int>>{{1, 5, 1}, {3, 3, 4}}, std::vector<int>{5, 4}),

    std::make_tuple(std::vector<std::vector<int>>{{42}}, std::vector<int>{42})};

const auto kTestTasksList = std::tuple_cat(ppc::util::AddFuncTask<RemizovKMaxInMatrixStringMPI, InType>(
                                               kTestParam, PPC_SETTINGS_remizov_k_max_in_matrix_string),
                                           ppc::util::AddFuncTask<RemizovKMaxInMatrixStringSEQ, InType>(
                                               kTestParam, PPC_SETTINGS_remizov_k_max_in_matrix_string));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = RemizovKRunFuncMaxInMatrixString::PrintFuncTestName<RemizovKRunFuncMaxInMatrixString>;

INSTANTIATE_TEST_SUITE_P(MatrixMaxTests, RemizovKRunFuncMaxInMatrixString, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace remizov_k_max_in_matrix_string
