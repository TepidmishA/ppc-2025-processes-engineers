#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <vector>

#include "util/include/perf_test_util.hpp"
// #include "util/include/util.hpp"
#include "viderman_a_elem_vec_sum/common/include/common.hpp"
#include "viderman_a_elem_vec_sum/mpi/include/ops_mpi.hpp"
#include "viderman_a_elem_vec_sum/seq/include/ops_seq.hpp"

namespace viderman_a_elem_vec_sum {

class VidermanAElemVecSumPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  InType input_data_;
  OutType expected_result_ = 0.0;

  void SetUp() override {
    const int base_size = 1000000;
    input_data_.resize(base_size);

    for (int i = 0; i < base_size; i++) {
      input_data_[i] = static_cast<double>((i % 100) + 1) + 0.5;
    }

    expected_result_ = 0.0;
    for (double val : input_data_) {
      expected_result_ += val;
    }

    input_data_.insert(input_data_.end(), input_data_.begin(), input_data_.end());
    expected_result_ *= 2.0;

    input_data_.insert(input_data_.end(), input_data_.begin(), input_data_.end());
    expected_result_ *= 2.0;

    input_data_.insert(input_data_.end(), input_data_.begin(), input_data_.end());
    expected_result_ *= 2.0;

    input_data_.insert(input_data_.end(), input_data_.begin(), input_data_.end());
    expected_result_ *= 2.0;

    input_data_.insert(input_data_.end(), input_data_.begin(), input_data_.end());
    expected_result_ *= 2.0;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    double relative_error = std::fabs(expected_result_ - output_data) / std::fabs(expected_result_);
    return relative_error < std::numeric_limits<double>::epsilon() * 1000;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(VidermanAElemVecSumPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, VidermanAElemVecSumMPI, VidermanAElemVecSumSEQ>(
    PPC_SETTINGS_viderman_a_elem_vec_sum);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = VidermanAElemVecSumPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(PerfTests, VidermanAElemVecSumPerfTests, kGtestValues, kPerfTestName);

}  // namespace viderman_a_elem_vec_sum
