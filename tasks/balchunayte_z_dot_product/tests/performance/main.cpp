#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "balchunayte_z_dot_product/common/include/common.hpp"
#include "balchunayte_z_dot_product/mpi/include/ops_mpi.hpp"
#include "balchunayte_z_dot_product/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace balchunayte_z_dot_product {

class DotProductRunPerfTestProcesses : public ppc::util::BaseRunPerfTests<InType, OutType> {
 public:
  void SetUp() override {
    constexpr int kSize = 5000000;

    input_data_.a.resize(kSize);
    input_data_.b.resize(kSize);

    for (int i = 0; i < kSize; ++i) {
      input_data_.a[i] = static_cast<double>(i + 1);
      input_data_.b[i] = static_cast<double>(2 * (i + 1));
    }

    expected_ = 0.0;
    for (int i = 0; i < kSize; ++i) {
      expected_ += input_data_.a[i] * input_data_.b[i];
    }
  }

  bool CheckTestOutputData(OutType &output_data) override {
    const double abs_eps = 1e-6;
    const double rel_eps = 1e-12;

    const double diff = std::fabs(output_data - expected_);
    const double scale = std::fabs(expected_);
    const double threshold = abs_eps + (rel_eps * scale);

    return diff <= threshold;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_{};
  OutType expected_{0.0};
};

TEST_P(DotProductRunPerfTestProcesses, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, BalchunayteZDotProductMPI, BalchunayteZDotProductSEQ>(
    PPC_SETTINGS_balchunayte_z_dot_product);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);
const auto kPerfTestName = DotProductRunPerfTestProcesses::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, DotProductRunPerfTestProcesses, kGtestValues, kPerfTestName);

}  // namespace balchunayte_z_dot_product
