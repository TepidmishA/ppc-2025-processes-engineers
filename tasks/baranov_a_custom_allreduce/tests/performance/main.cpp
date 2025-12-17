#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <exception>
#include <random>
#include <string>
#include <vector>

#include "baranov_a_custom_allreduce/common/include/common.hpp"
#include "baranov_a_custom_allreduce/mpi/include/ops_mpi.hpp"
#include "baranov_a_custom_allreduce/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace baranov_a_custom_allreduce {

class BaranovACustomAllreducePerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
 protected:
  void SetUp() override {
    auto param = GetParam();
    std::string task_name = std::get<1>(param);
    is_mpi_test_ = (task_name.find("mpi") != std::string::npos);

    int size = 10000000;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1000.0, 1000.0);

    std::vector<double> data(size);
    for (int i = 0; i < size; i++) {
      data[i] = dis(gen);
    }

    input_data_ = InTypeVariant{data};
  }

  bool CheckTestOutputData(OutType &output_data) final {
    try {
      if (!is_mpi_test_) {
        return output_data == input_data_;
      }
      auto output_vec = std::get<std::vector<double>>(output_data);
      bool has_invalid =
          std::ranges::any_of(output_vec, [](const auto &val) { return std::isnan(val) || std::isinf(val); });

      return !has_invalid;
    } catch (const std::exception &) {
      return false;
    }
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  bool is_mpi_test_ = false;
};

TEST_P(BaranovACustomAllreducePerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, BaranovACustomAllreduceMPI, BaranovACustomAllreduceSEQ>(
    PPC_SETTINGS_baranov_a_custom_allreduce);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = BaranovACustomAllreducePerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, BaranovACustomAllreducePerfTests, kGtestValues, kPerfTestName);

}  // namespace baranov_a_custom_allreduce
