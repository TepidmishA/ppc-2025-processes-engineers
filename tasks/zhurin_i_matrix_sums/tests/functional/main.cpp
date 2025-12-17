#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <tuple>
#include <vector>

#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"
#include "zhurin_i_matrix_sums/common/include/common.hpp"
#include "zhurin_i_matrix_sums/mpi/include/ops_mpi.hpp"
#include "zhurin_i_matrix_sums/seq/include/ops_seq.hpp"

namespace zhurin_i_matrix_sums {

class ZhurinIRunFuncTestsProcesses : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return test_param;
  }

 protected:
  void SetUp() override {
    TestType param = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    std::string input_data_source =
        ppc::util::GetAbsoluteTaskPath(PPC_ID_zhurin_i_matrix_sums, "cases/" + param + ".txt");
    std::string expected_data_source =
        ppc::util::GetAbsoluteTaskPath(PPC_ID_zhurin_i_matrix_sums, "expected/" + param + ".txt");

    std::ifstream file(input_data_source);
    uint32_t rows = 0;
    uint32_t columns = 0;
    std::vector<double> inp;
    double num = 0.0;

    file >> rows;
    file >> columns;

    while (file >> num) {
      inp.push_back(num);
    }
    file.close();

    file = std::ifstream(expected_data_source);
    double expected_value = 0.0;
    file >> expected_value;
    file.close();

    input_data_ = InType(rows, columns, inp);
    expected_data_ = expected_value;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return std::abs(output_data - expected_data_) < kEpsilon;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  OutType expected_data_{0.0};
};

namespace {

TEST_P(ZhurinIRunFuncTestsProcesses, MatrixSums) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 10> kTestParam = {"test1", "test2", "test3", "test4", "test5",
                                             "test6", "test7", "test8", "test9", "test10"};

const auto kTestTasksList = std::tuple_cat(ppc::util::AddFuncTask<zhurin_i_matrix_sums::ZhurinIMatrixSumsMPI, InType>(
                                               kTestParam, PPC_SETTINGS_zhurin_i_matrix_sums),
                                           ppc::util::AddFuncTask<zhurin_i_matrix_sums::ZhurinIMatrixSumsSEQ, InType>(
                                               kTestParam, PPC_SETTINGS_zhurin_i_matrix_sums));

inline const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

inline const auto kPerfTestName = ZhurinIRunFuncTestsProcesses::PrintFuncTestName<ZhurinIRunFuncTestsProcesses>;

// NOLINTNEXTLINE
INSTANTIATE_TEST_SUITE_P(ZhurinIMatrix, ZhurinIRunFuncTestsProcesses, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace zhurin_i_matrix_sums
