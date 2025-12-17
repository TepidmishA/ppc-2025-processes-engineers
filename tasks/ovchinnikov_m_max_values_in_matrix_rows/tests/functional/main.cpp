#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <string>
#include <tuple>
#include <vector>

#include "ovchinnikov_m_max_values_in_matrix_rows/common/include/common.hpp"
#include "ovchinnikov_m_max_values_in_matrix_rows/mpi/include/ops_mpi.hpp"
#include "ovchinnikov_m_max_values_in_matrix_rows/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace ovchinnikov_m_max_values_in_matrix_rows {

class OvchinnikovMMaxValuesInMatrixRowsFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::get<3>(test_param);  // name
  }

 protected:
  void SetUp() override {
    const TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());

    size_t rows = std::get<0>(params);
    size_t cols = std::get<1>(params);
    const std::vector<int> &data = std::get<2>(params);

    input_data_ = std::make_tuple(rows, cols, data);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    OutType expected = CalcExpected(input_data_);
    return output_data == expected;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;

  static OutType CalcExpected(const InType &m) {
    size_t rows = std::get<0>(m);
    size_t cols = std::get<1>(m);
    const std::vector<int> &data = std::get<2>(m);

    if (rows == 0 || cols == 0) {
      return {};
    }

    OutType result(cols, std::numeric_limits<int>::min());

    for (size_t i = 0; i < rows; i++) {
      for (size_t j = 0; j < cols; j++) {
        int value = data[(i * cols) + j];
        result[j] = std::max(result[j], value);
      }
    }

    return result;
  }
};

namespace {

TEST_P(OvchinnikovMMaxValuesInMatrixRowsFuncTests, MatmulFromPic) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 6> kTestParam = {
    std::make_tuple(0, 0, std::vector<int>{}, "empty_matrix"),

    std::make_tuple(3, 3, std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9}, "matrix_3x3"),

    std::make_tuple(2, 2, std::vector<int>{-1, -5, 4, 0}, "negatives"),

    std::make_tuple(1, 1, std::vector<int>{10}, "single_element"),

    std::make_tuple(2, 3, std::vector<int>{1, 10, 3, 7, 0, 100}, "random"),

    std::make_tuple(5, 9, std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 7, 6, 3, 2, 9, 4, 2, 5, 8, 1, 2, 3, 4, 5,
                                           6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 7, 6, 3, 2, 9, 4, 2, 5, 8},
                    "large_matrix"),
};

const auto kTestTasksList = std::tuple_cat(ppc::util::AddFuncTask<OvchinnikovMMaxValuesInMatrixRowsMPI, InType>(
                                               kTestParam, PPC_SETTINGS_ovchinnikov_m_max_values_in_matrix_rows),
                                           ppc::util::AddFuncTask<OvchinnikovMMaxValuesInMatrixRowsSEQ, InType>(
                                               kTestParam, PPC_SETTINGS_ovchinnikov_m_max_values_in_matrix_rows));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName =
    OvchinnikovMMaxValuesInMatrixRowsFuncTests::PrintFuncTestName<OvchinnikovMMaxValuesInMatrixRowsFuncTests>;

INSTANTIATE_TEST_SUITE_P(MaxValuesTests, OvchinnikovMMaxValuesInMatrixRowsFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace ovchinnikov_m_max_values_in_matrix_rows
