#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "chernov_t_max_matrix_columns/common/include/common.hpp"
#include "chernov_t_max_matrix_columns/mpi/include/ops_mpi.hpp"
#include "chernov_t_max_matrix_columns/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace chernov_t_max_matrix_columns {

class ChernovTFuncTestsProcesses : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::get<0>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    GetDataFromFile(params);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    auto expected =
        std::get<2>(std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam()));

    if (output_data.size() != expected.size()) {
      return false;
    }

    for (std::size_t i = 0; i < output_data.size(); ++i) {
      if (output_data[i] != expected[i]) {
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

  void GetDataFromFile(const TestType &params) {
    std::string filename = std::get<1>(params);
    std::string abs_path = ppc::util::GetAbsoluteTaskPath(PPC_ID_chernov_t_max_matrix_columns, filename);

    std::ifstream file(abs_path);
    if (!file.is_open()) {
      throw std::runtime_error("Failed to open file: " + abs_path);
    }

    std::size_t rows = 0;
    std::size_t cols = 0;
    file >> rows >> cols;

    std::vector<int> matrix_data(rows * cols);
    for (std::size_t i = 0; i < matrix_data.size(); i++) {
      file >> matrix_data[i];
    }

    input_data_ = std::make_tuple(rows, cols, matrix_data);
  }
};

namespace {

TEST_P(ChernovTFuncTestsProcesses, MaxMatrixColumns) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 2> kTestParam = {std::make_tuple("Matrix_3x3", "matrix_1.txt", std::vector<int>({7, 8, 9})),
                                            std::make_tuple("Matrix_2x2", "matrix_2.txt", std::vector<int>({4, 3}))};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<ChernovTMaxMatrixColumnsMPI, InType>(kTestParam, PPC_SETTINGS_chernov_t_max_matrix_columns),
    ppc::util::AddFuncTask<ChernovTMaxMatrixColumnsSEQ, InType>(kTestParam, PPC_SETTINGS_chernov_t_max_matrix_columns));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = ChernovTFuncTestsProcesses::PrintFuncTestName<ChernovTFuncTestsProcesses>;

INSTANTIATE_TEST_SUITE_P(MatrixColumnsTests, ChernovTFuncTestsProcesses, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace chernov_t_max_matrix_columns
