#include <gtest/gtest.h>

#include <cmath>
#include <numbers>

#include "galkin_d_trapezoid_method/common/include/common.hpp"
#include "galkin_d_trapezoid_method/mpi/include/ops_mpi.hpp"
#include "galkin_d_trapezoid_method/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

constexpr double kPi = std::numbers::pi;

namespace galkin_d_trapezoid_method {

class GalkinDTrapezoidPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
 protected:
  void SetUp() override {
    constexpr int kCount = 2'000'000;

    input_data_ = InType{.a = 0.0, .b = kPi, .n = kCount, .func_id = static_cast<int>(FunctionId::kSin)};
  }

  bool CheckTestOutputData(OutType &output_data) final {
    const double exact = GetExactIntegral(input_data_);
    constexpr double kEps = 1e-4;

    return std::fabs(output_data - exact) < kEps;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_{.a = 0.0, .b = 1.0, .n = 10, .func_id = static_cast<int>(FunctionId::kLinear)};
};

namespace {

TEST_P(GalkinDTrapezoidPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, GalkinDTrapezoidMethodMPI, GalkinDTrapezoidMethodSEQ>(
    PPC_SETTINGS_galkin_d_trapezoid_method);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = GalkinDTrapezoidPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, GalkinDTrapezoidPerfTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace galkin_d_trapezoid_method
