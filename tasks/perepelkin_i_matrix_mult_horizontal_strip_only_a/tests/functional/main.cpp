#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "perepelkin_i_matrix_mult_horizontal_strip_only_a/common/include/common.hpp"
#include "perepelkin_i_matrix_mult_horizontal_strip_only_a/mpi/include/ops_mpi.hpp"
#include "perepelkin_i_matrix_mult_horizontal_strip_only_a/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace perepelkin_i_matrix_mult_horizontal_strip_only_a {

class PerepelkinIMatrixMultHorizontalStripOnlyAFuncTestProcesses
    : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::get<0>(test_param);
  }

 protected:
  void SetUp() override {
    const auto &[test_name, matrix_A, matrix_B, matrix_C] =
        std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    input_data_ = std::make_pair(matrix_A, matrix_B);
    expected_ = matrix_C;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return expected_ == output_data;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  OutType expected_;
};

namespace {

TEST_P(PerepelkinIMatrixMultHorizontalStripOnlyAFuncTestProcesses, StringDifFromFile) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 16> kTestParam = {
    std::make_tuple("basic_sample", std::vector<std::vector<double>>{{1.0, 2.0}, {3.0, 4.0}},
                    std::vector<std::vector<double>>{{5.0, 6.0}, {7.0, 8.0}},
                    std::vector<std::vector<double>>{{19.0, 22.0}, {43.0, 50.0}}),

    std::make_tuple("nontrivial_mixed",
                    std::vector<std::vector<double>>{
                        {1.0, 2.0, 3.0, 4.0}, {0.0, 1.0, 0.0, 1.0}, {2.0, 0.0, 1.0, 0.0}, {1.0, 1.0, 1.0, 1.0}},
                    std::vector<std::vector<double>>{
                        {1.0, 0.0, 1.0, 0.0}, {0.0, 1.0, 0.0, 1.0}, {1.0, 1.0, 0.0, 0.0}, {0.0, 0.0, 1.0, 1.0}},
                    std::vector<std::vector<double>>{
                        {4.0, 5.0, 5.0, 6.0}, {0.0, 1.0, 1.0, 2.0}, {3.0, 1.0, 2.0, 0.0}, {2.0, 2.0, 2.0, 2.0}}),

    std::make_tuple("zeros_both", std::vector<std::vector<double>>{{0.0, 0.0}, {0.0, 0.0}},
                    std::vector<std::vector<double>>{{0.0, 0.0}, {0.0, 0.0}},
                    std::vector<std::vector<double>>{{0.0, 0.0}, {0.0, 0.0}}),

    std::make_tuple("zeros_only_a", std::vector<std::vector<double>>{{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}},
                    std::vector<std::vector<double>>{{1.0, 2.0}, {3.0, 4.0}},
                    std::vector<std::vector<double>>{{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}}),

    std::make_tuple(
        "zeros_only_b",
        std::vector<std::vector<double>>{{1.0, 2.0, 3.0, 4.0}, {0.0, -1.0, 2.0, 1.0}, {5.0, 0.0, 1.0, -2.0}},
        std::vector<std::vector<double>>{{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}},
        std::vector<std::vector<double>>{{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}}),

    std::make_tuple("identity_both", std::vector<std::vector<double>>{{1.0, 0.0}, {0.0, 1.0}},
                    std::vector<std::vector<double>>{{1.0, 0.0}, {0.0, 1.0}},
                    std::vector<std::vector<double>>{{1.0, 0.0}, {0.0, 1.0}}),

    std::make_tuple("identity_only_a",
                    std::vector<std::vector<double>>{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}},
                    std::vector<std::vector<double>>{{2.0, 3.0, 4.0}, {5.0, 6.0, 7.0}, {8.0, 9.0, 10.0}},
                    std::vector<std::vector<double>>{{2.0, 3.0, 4.0}, {5.0, 6.0, 7.0}, {8.0, 9.0, 10.0}}),

    std::make_tuple("identity_only_b",
                    std::vector<std::vector<double>>{{1000.0, 2000.0, 3000.0}, {0.0, 1.0, 0.0}, {-1.0, 2.0, -3.0}},
                    std::vector<std::vector<double>>{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}},
                    std::vector<std::vector<double>>{{1000.0, 2000.0, 3000.0}, {0.0, 1.0, 0.0}, {-1.0, 2.0, -3.0}}),

    std::make_tuple(
        "row_times_matrix", std::vector<std::vector<double>>{{1.0, -1.0, 2.0, 0.5}},
        std::vector<std::vector<double>>{{1.0, 0.0, 2.0}, {0.0, 1.0, -1.0}, {2.0, 2.0, 0.0}, {-1.0, 0.5, 1.0}},
        std::vector<std::vector<double>>{{4.5, 3.25, 3.5}}),

    std::make_tuple(
        "col_times_row", std::vector<std::vector<double>>{{1.0}, {2.0}, {-1.0}, {0.0}},
        std::vector<std::vector<double>>{{3.0, -2.0, 0.0}},
        std::vector<std::vector<double>>{{3.0, -2.0, 0.0}, {6.0, -4.0, 0.0}, {-3.0, 2.0, 0.0}, {0.0, 0.0, 0.0}}),

    std::make_tuple("row_times_col", std::vector<std::vector<double>>{{1.0, 2.0, 3.0}},
                    std::vector<std::vector<double>>{{4.0}, {5.0}, {6.0}}, std::vector<std::vector<double>>{{32.0}}),

    std::make_tuple("single_elem", std::vector<std::vector<double>>{{7.0}}, std::vector<std::vector<double>>{{3.0}},
                    std::vector<std::vector<double>>{{21.0}}),

    std::make_tuple("uneven_rows",
                    std::vector<std::vector<double>>{
                        {1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}, {1.0, 0.0, 1.0}, {2.0, 2.0, 2.0}},
                    std::vector<std::vector<double>>{{1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}},
                    std::vector<std::vector<double>>{{4.0, 5.0}, {10.0, 11.0}, {16.0, 17.0}, {2.0, 1.0}, {4.0, 4.0}}),

    std::make_tuple("negative_values", std::vector<std::vector<double>>{{-1.0, -2.0}, {-3.0, -4.0}},
                    std::vector<std::vector<double>>{{1.0, -1.0}, {2.0, 0.0}},
                    std::vector<std::vector<double>>{{-5.0, 1.0}, {-11.0, 3.0}}),

    std::make_tuple("alternating_signs", std::vector<std::vector<double>>{{1.0, -1.0, 1.0}, {-1.0, 1.0, -1.0}},
                    std::vector<std::vector<double>>{{1.0, 2.0}, {3.0, 4.0}, {5.0, 6.0}},
                    std::vector<std::vector<double>>{{3.0, 4.0}, {-3.0, -4.0}}),

    std::make_tuple("mixed_signs", std::vector<std::vector<double>>{{2.0, -1.0}, {0.0, 3.0}, {-2.0, 4.0}},
                    std::vector<std::vector<double>>{{1.0, 2.0, -1.0}, {3.0, -2.0, 0.5}},
                    std::vector<std::vector<double>>{{-1.0, 6.0, -2.5}, {9.0, -6.0, 1.5}, {10.0, -12.0, 4.0}})};

const auto kTestTasksList =
    std::tuple_cat(ppc::util::AddFuncTask<PerepelkinIMatrixMultHorizontalStripOnlyAMPI, InType>(
                       kTestParam, PPC_SETTINGS_perepelkin_i_matrix_mult_horizontal_strip_only_a),
                   ppc::util::AddFuncTask<PerepelkinIMatrixMultHorizontalStripOnlyASEQ, InType>(
                       kTestParam, PPC_SETTINGS_perepelkin_i_matrix_mult_horizontal_strip_only_a));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kFuncTestName = PerepelkinIMatrixMultHorizontalStripOnlyAFuncTestProcesses::PrintFuncTestName<
    PerepelkinIMatrixMultHorizontalStripOnlyAFuncTestProcesses>;

INSTANTIATE_TEST_SUITE_P(PicMatrixTests, PerepelkinIMatrixMultHorizontalStripOnlyAFuncTestProcesses, kGtestValues,
                         kFuncTestName);

}  // namespace

}  // namespace perepelkin_i_matrix_mult_horizontal_strip_only_a
