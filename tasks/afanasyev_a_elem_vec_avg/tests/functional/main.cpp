#include <gtest/gtest.h>
#include <mpi.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <random>
#include <string>
#include <tuple>

#include "afanasyev_a_elem_vec_avg/common/include/common.hpp"
#include "afanasyev_a_elem_vec_avg/mpi/include/ops_mpi.hpp"
#include "afanasyev_a_elem_vec_avg/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace afanasyev_a_elem_vec_avg {

using TestType = std::tuple<int, std::string>;

class AfanasyevAElemVecAvgFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return "Size_" + std::to_string(std::get<0>(test_param)) + "_" + std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    int vector_size = std::get<0>(params);

    if (vector_size <= 0) {
      input_data_ = {};
      expected_output_ = 0.0;
    } else {
      input_data_.resize(vector_size);

      unsigned int seed = 0;
      int is_mpi_initialized = 0;
      MPI_Initialized(&is_mpi_initialized);

      if (is_mpi_initialized != 0) {
        int rank = 0;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        if (rank == 0) {
          std::random_device rd;
          seed = rd();
        }
        MPI_Bcast(&seed, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
      } else {
        std::random_device rd;
        seed = rd();
      }

      std::mt19937 gen(seed);
      std::uniform_int_distribution<> distrib(-1000, 1000);

      for (int i = 0; i < vector_size; ++i) {
        input_data_[i] = distrib(gen);
      }

      int64_t sum = std::accumulate(input_data_.begin(), input_data_.end(), static_cast<int64_t>(0));
      expected_output_ = static_cast<double>(sum) / vector_size;
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    const double tolerance = 1e-5;
    return std::abs(output_data - expected_output_) < tolerance;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  OutType expected_output_ = 0.0;
};

namespace {

TEST_P(AfanasyevAElemVecAvgFuncTests, CalculateAverage) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 5> kTestParam = {std::make_tuple(100, "Normal"), std::make_tuple(10, "Small"),
                                            std::make_tuple(0, "EmptyVector"), std::make_tuple(1, "SingleElement"),
                                            std::make_tuple(10000, "Large")};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<AfanasyevAElemVecAvgMPI, InType>(kTestParam, PPC_SETTINGS_afanasyev_a_elem_vec_avg),
    ppc::util::AddFuncTask<AfanasyevAElemVecAvgSEQ, InType>(kTestParam, PPC_SETTINGS_afanasyev_a_elem_vec_avg));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = AfanasyevAElemVecAvgFuncTests::PrintFuncTestName<AfanasyevAElemVecAvgFuncTests>;

INSTANTIATE_TEST_SUITE_P(VectorAverageTests, AfanasyevAElemVecAvgFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace afanasyev_a_elem_vec_avg
