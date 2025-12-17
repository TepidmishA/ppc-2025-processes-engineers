#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "remizov_k_max_in_matrix_string/common/include/common.hpp"
#include "remizov_k_max_in_matrix_string/mpi/include/ops_mpi.hpp"
#include "remizov_k_max_in_matrix_string/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace remizov_k_max_in_matrix_string {

class RemizovKMaxInMatrixStringPerfTest : public ppc::util::BaseRunPerfTests<InType, OutType> {
  const int kCount_ = 50000000;
  InType input_data_;
  OutType res_;

  void SetUp() override {
    int n = static_cast<int>(std::sqrt(kCount_));
    input_data_ = std::vector<std::vector<int>>(n, std::vector<int>(n, 2));
    res_ = std::vector<int>(n, 2);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return res_ == output_data;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(RemizovKMaxInMatrixStringPerfTest, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, RemizovKMaxInMatrixStringMPI, RemizovKMaxInMatrixStringSEQ>(
        PPC_SETTINGS_remizov_k_max_in_matrix_string);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = RemizovKMaxInMatrixStringPerfTest::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, RemizovKMaxInMatrixStringPerfTest, kGtestValues, kPerfTestName);

}  // namespace remizov_k_max_in_matrix_string
