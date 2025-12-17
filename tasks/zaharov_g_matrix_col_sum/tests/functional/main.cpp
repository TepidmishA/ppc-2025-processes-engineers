#include <gtest/gtest.h>
#include <stb/stb_image.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"
#include "zaharov_g_matrix_col_sum/common/include/common.hpp"
#include "zaharov_g_matrix_col_sum/mpi/include/ops_mpi.hpp"
#include "zaharov_g_matrix_col_sum/seq/include/ops_seq.hpp"

namespace zaharov_g_matrix_col_sum {

class ZaharovGRunFuncTestsProcesses : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param)) + "_" + std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    int test_case = std::get<0>(params);

    switch (test_case) {
      case 1:
        input_data_ = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}};
        expected_sums_ = {12.0, 15.0, 18.0};
        break;

      case 2:
        input_data_ = {{1.0, 2.0, 3.0, 4.0, 5.0},
                       {1.0, 2.0, 3.0, 4.0, 5.0},
                       {1.0, 2.0, 3.0, 4.0, 5.0},
                       {1.0, 2.0, 3.0, 4.0, 5.0},
                       {1.0, 2.0, 3.0, 4.0, 5.0}};
        expected_sums_ = {5.0, 10.0, 15.0, 20.0, 25.0};
        break;

      case 3:
        input_data_ = {{1.0, 2.0, 3.0, 4.0}, {5.0, 6.0, 7.0, 8.0}};
        expected_sums_ = {6.0, 8.0, 10.0, 12.0};
        break;

      case 4:
        input_data_ = {{1.0, 2.0}, {3.0, 4.0}, {5.0, 6.0}, {7.0, 8.0}};
        expected_sums_ = {16.0, 20.0};
        break;

      case 5:
        input_data_ = {
            {1.0, 2.0, 3.0, 4.0, 5.0, 6.0}, {2.0, 4.0, 6.0, 8.0, 10.0, 12.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}};
        expected_sums_ = {4.0, 7.0, 10.0, 13.0, 16.0, 19.0};
        break;

      case 6:
        input_data_ = {{10.0, 20.0, 30.0, 40.0, 50.0}};
        expected_sums_ = {10.0, 20.0, 30.0, 40.0, 50.0};
        break;

      case 7:
        input_data_ = {{2.0}, {4.0}, {6.0}, {8.0}, {10.0}};
        expected_sums_ = {30.0};
        break;

      default:
        throw std::runtime_error("Unsupported test case: " + std::to_string(test_case));
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    if (output_data.size() != expected_sums_.size()) {
      return false;
    }

    for (size_t i = 0; i < output_data.size(); i++) {
      if (std::abs(output_data[i] - expected_sums_[i]) > 1e-9) {
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
  std::vector<double> expected_sums_;
};

namespace {

TEST_P(ZaharovGRunFuncTestsProcesses, MatrixColSumTests) {
  ExecuteTest(GetParam());
}

// Тестовые параметры: номер теста и описание
const std::array<TestType, 7> kTestParam = {
    std::make_tuple(1, "3x3"), std::make_tuple(2, "5x5"), std::make_tuple(3, "2x4"), std::make_tuple(4, "4x2"),
    std::make_tuple(5, "3x6"), std::make_tuple(6, "1x5"), std::make_tuple(7, "5x1")};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<ZaharovGMatrixColSumMPI, InType>(kTestParam, PPC_SETTINGS_zaharov_g_matrix_col_sum),
    ppc::util::AddFuncTask<ZaharovGMatrixColSumSEQ, InType>(kTestParam, PPC_SETTINGS_zaharov_g_matrix_col_sum));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = ZaharovGRunFuncTestsProcesses::PrintFuncTestName<ZaharovGRunFuncTestsProcesses>;

INSTANTIATE_TEST_SUITE_P(MatrixColSumTests, ZaharovGRunFuncTestsProcesses, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace zaharov_g_matrix_col_sum
