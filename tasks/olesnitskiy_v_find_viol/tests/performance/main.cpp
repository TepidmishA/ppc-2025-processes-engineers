#include <gtest/gtest.h>

#include <utility>
#include <vector>

#include "olesnitskiy_v_find_viol/common/include/common.hpp"
#include "olesnitskiy_v_find_viol/mpi/include/ops_mpi.hpp"
#include "olesnitskiy_v_find_viol/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace olesnitskiy_v_find_viol {

class OlesnitskiyVFindViolPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  InType input_data_;

  void SetUp() override {
    const int vector_size = 90000000;
    std::vector<double> vector(vector_size);

    for (int i = 0; i < vector_size; i++) {
      vector[i] = static_cast<double>(i % 1000) + ((i & 1) == 0 ? 1.0 : -1.0);
    }
    input_data_ = vector;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return output_data >= 0 && std::cmp_less_equal(output_data, input_data_.size());
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(OlesnitskiyVFindViolPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, OlesnitskiyVFindViolMPI, OlesnitskiyVFindViolSEQ>(
    PPC_SETTINGS_olesnitskiy_v_find_viol);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = OlesnitskiyVFindViolPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, OlesnitskiyVFindViolPerfTests, kGtestValues, kPerfTestName);

}  // namespace olesnitskiy_v_find_viol
