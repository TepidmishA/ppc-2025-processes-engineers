#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <string>

#include "shakirova_e_elem_matrix_sum/common/include/common.hpp"
#include "shakirova_e_elem_matrix_sum/mpi/include/ops_mpi.hpp"
#include "shakirova_e_elem_matrix_sum/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"
#include "util/include/util.hpp"

namespace shakirova_e_elem_matrix_sum {

class ShakirovaEElemMatrixSumPerfTest : public ppc::util::BaseRunPerfTests<InType, OutType> {
 protected:
  void SetUp() override {
    if (ShouldInitializeTestData()) {
      InitializeTestData();
      return;
    }

    SetEmptyData();
  }

  bool CheckTestOutputData(OutType &output_data) final {
    if (ShouldInitializeTestData()) {
      return output_data_ == output_data;
    }
    return true;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  const size_t matrix_size_ = 16000;
  InType input_data_ = {};
  OutType output_data_ = 0;

  [[nodiscard]] static bool ShouldInitializeTestData() {
    const std::string &test_type =
        std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kNameTest)>(GetParam());

    if (test_type.find("_mpi") == std::string::npos) {
      return true;
    }

    return !ppc::util::IsUnderMpirun() || ppc::util::GetMPIRank() == 0;
  }

  void SetEmptyData() {
    input_data_ = {.rows = 0, .cols = 0, .data = {}};
    output_data_ = 0;
  }

  void InitializeTestData() {
    input_data_.rows = matrix_size_;
    input_data_.cols = matrix_size_;
    input_data_.data.assign(matrix_size_ * matrix_size_, 1);
    output_data_ = static_cast<int64_t>(matrix_size_) * static_cast<int64_t>(matrix_size_);
  }
};

TEST_P(ShakirovaEElemMatrixSumPerfTest, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, ShakirovaEElemMatrixSumMPI, ShakirovaEElemMatrixSumSEQ>(
    PPC_SETTINGS_shakirova_e_elem_matrix_sum);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = ShakirovaEElemMatrixSumPerfTest::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, ShakirovaEElemMatrixSumPerfTest, kGtestValues, kPerfTestName);

}  // namespace shakirova_e_elem_matrix_sum
