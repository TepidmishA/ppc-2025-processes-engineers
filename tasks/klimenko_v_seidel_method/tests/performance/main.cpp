#include <gtest/gtest.h>

#include "klimenko_v_seidel_method/common/include/common.hpp"
#include "klimenko_v_seidel_method/mpi/include/ops_mpi.hpp"
#include "klimenko_v_seidel_method/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace klimenko_v_seidel_method {

class KlimenkoVSeidelMethodPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  const int kCount_ = 2500;
  InType input_data_{};

  void SetUp() override {
    input_data_ = kCount_;
  }

  bool CheckTestOutputData(OutType &output_data) override {
    return input_data_ == output_data;
  }

  InType GetTestInputData() override {
    return input_data_;
  }
};

TEST_P(KlimenkoVSeidelMethodPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, KlimenkoVSeidelMethodMPI, KlimenkoVSeidelMethodSEQ>(
    PPC_SETTINGS_klimenko_v_seidel_method);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = KlimenkoVSeidelMethodPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, KlimenkoVSeidelMethodPerfTests, kGtestValues, kPerfTestName);

}  // namespace klimenko_v_seidel_method
