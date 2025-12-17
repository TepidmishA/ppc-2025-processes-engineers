#include <gtest/gtest.h>

#include <random>

#include "sinev_a_min_in_vector/common/include/common.hpp"
#include "sinev_a_min_in_vector/mpi/include/ops_mpi.hpp"
#include "sinev_a_min_in_vector/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace sinev_a_min_in_vector {

class SinevAMinInVectorPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  int real_min_{};
  InType input_data_;

  void SetUp() override {
    int size = 100000000;
    input_data_.resize(size);

    // Заполняем сложными данными
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 1000000);

    for (int i = 0; i < size; i++) {
      input_data_[i] = dist(gen);
    }

    // Добавляем явный минимум
    input_data_[size / 2] = -1000;
    real_min_ = -1000;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return output_data == real_min_;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(SinevAMinInVectorPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, SinevAMinInVectorMPI, SinevAMinInVectorSEQ>(PPC_SETTINGS_sinev_a_min_in_vector);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = SinevAMinInVectorPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, SinevAMinInVectorPerfTests, kGtestValues, kPerfTestName);

}  // namespace sinev_a_min_in_vector
