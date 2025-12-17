#include <gtest/gtest.h>
#include <mpi.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <random>
#include <string>
#include <tuple>

#include "dilshodov_a_max_val_rows_matrix/common/include/common.hpp"
#include "dilshodov_a_max_val_rows_matrix/mpi/include/ops_mpi.hpp"
#include "dilshodov_a_max_val_rows_matrix/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace dilshodov_a_max_val_rows_matrix {

class DilshodovMaxValRowsMatrixFuncTest : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param)) + "x" + std::to_string(std::get<1>(test_param)) + "_" +
           std::get<2>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    int rows = std::get<0>(params);
    int cols = std::get<1>(params);

    input_data_.resize(rows);
    for (int i = 0; i < rows; ++i) {
      input_data_[i].resize(cols);
    }

    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(-1000, 1000);

    for (int i = 0; i < rows; ++i) {
      for (int j = 0; j < cols; ++j) {
        input_data_[i][j] = dist(gen);
      }
    }

    expected_output_.resize(rows);
    for (int i = 0; i < rows; ++i) {
      expected_output_[i] = *std::max_element(input_data_[i].begin(), input_data_[i].end());
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    if (ppc::util::IsUnderMpirun()) {
      int rank = 0;
      MPI_Comm_rank(MPI_COMM_WORLD, &rank);
      if (rank != 0) {
        return true;
      }
    }
    return output_data == expected_output_;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  OutType expected_output_;
};

namespace {

TEST_P(DilshodovMaxValRowsMatrixFuncTest, MaxValRowsMatrix) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 6> kTestParam = {
    std::make_tuple(1, 1, "1x1"),   std::make_tuple(3, 3, "3x3"),         std::make_tuple(5, 10, "5x10"),
    std::make_tuple(10, 5, "10x5"), std::make_tuple(100, 100, "100x100"), std::make_tuple(50, 200, "50x200"),
};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<MaxValRowsMatrixTaskMPI, InType>(kTestParam, PPC_SETTINGS_dilshodov_a_max_val_rows_matrix),
    ppc::util::AddFuncTask<MaxValRowsMatrixTaskSequential, InType>(kTestParam,
                                                                   PPC_SETTINGS_dilshodov_a_max_val_rows_matrix));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kFuncTestName = DilshodovMaxValRowsMatrixFuncTest::PrintFuncTestName<DilshodovMaxValRowsMatrixFuncTest>;

INSTANTIATE_TEST_SUITE_P(MaxValRowsMatrixTests, DilshodovMaxValRowsMatrixFuncTest, kGtestValues, kFuncTestName);

}  // namespace

}  // namespace dilshodov_a_max_val_rows_matrix
