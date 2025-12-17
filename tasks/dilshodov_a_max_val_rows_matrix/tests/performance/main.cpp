#include <gtest/gtest.h>
#include <mpi.h>

#include <algorithm>
#include <random>

#include "dilshodov_a_max_val_rows_matrix/common/include/common.hpp"
#include "dilshodov_a_max_val_rows_matrix/mpi/include/ops_mpi.hpp"
#include "dilshodov_a_max_val_rows_matrix/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"
#include "util/include/util.hpp"

namespace dilshodov_a_max_val_rows_matrix {

class DilshodovMaxValRowsMatrixPerfTest : public ppc::util::BaseRunPerfTests<InType, OutType> {
  static constexpr int kRows = 5000;
  static constexpr int kCols = 5000;
  InType input_data_;
  OutType expected_output_;

  void SetUp() override {
    input_data_.resize(kRows);
    for (int i = 0; i < kRows; ++i) {
      input_data_[i].resize(kCols);
    }

    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(-10000, 10000);

    for (int i = 0; i < kRows; ++i) {
      for (int j = 0; j < kCols; ++j) {
        input_data_[i][j] = dist(gen);
      }
    }

    expected_output_.resize(kRows);
    for (int i = 0; i < kRows; ++i) {
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
};

TEST_P(DilshodovMaxValRowsMatrixPerfTest, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, MaxValRowsMatrixTaskMPI, MaxValRowsMatrixTaskSequential>(
    PPC_SETTINGS_dilshodov_a_max_val_rows_matrix);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = DilshodovMaxValRowsMatrixPerfTest::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, DilshodovMaxValRowsMatrixPerfTest, kGtestValues, kPerfTestName);

}  // namespace dilshodov_a_max_val_rows_matrix
