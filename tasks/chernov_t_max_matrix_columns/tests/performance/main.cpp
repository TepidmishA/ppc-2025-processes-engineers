#include <gtest/gtest.h>

#include <cstddef>
#include <tuple>
#include <vector>

#include "chernov_t_max_matrix_columns/common/include/common.hpp"
#include "chernov_t_max_matrix_columns/mpi/include/ops_mpi.hpp"
#include "chernov_t_max_matrix_columns/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace chernov_t_max_matrix_columns {

class ChernovTPerfTest : public ppc::util::BaseRunPerfTests<InType, OutType> {
 private:
  const std::size_t kRows_ = 7000;
  const std::size_t kCols_ = 7000;
  InType input_data_;

  void SetUp() override {
    std::vector<int> matrix_data(kRows_ * kCols_);

    for (std::size_t i = 0; i < kRows_; ++i) {
      for (std::size_t j = 0; j < kCols_; ++j) {
        int value = static_cast<int>(((i * 13 + j * 29) % 1000) + 1);
        matrix_data[(i * kCols_) + j] = value;
      }
    }
    input_data_ = std::make_tuple(kRows_, kCols_, matrix_data);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return !output_data.empty() && output_data.size() == kCols_;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(ChernovTPerfTest, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, ChernovTMaxMatrixColumnsMPI, ChernovTMaxMatrixColumnsSEQ>(
        PPC_SETTINGS_chernov_t_max_matrix_columns);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = ChernovTPerfTest::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(ChernovTPerfTests, ChernovTPerfTest, kGtestValues, kPerfTestName);

}  // namespace chernov_t_max_matrix_columns
