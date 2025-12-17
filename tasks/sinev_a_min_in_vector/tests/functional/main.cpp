#include <gtest/gtest.h>
#include <stb/stb_image.h>

#include <array>
#include <cstddef>
#include <string>
#include <tuple>

#include "sinev_a_min_in_vector/common/include/common.hpp"
#include "sinev_a_min_in_vector/mpi/include/ops_mpi.hpp"
#include "sinev_a_min_in_vector/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace sinev_a_min_in_vector {

class SinevAMinInVectorFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param)) + "_" + std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    int test_case = std::get<0>(params);

    switch (test_case) {
      case 0:
        input_data_ = {5, 3, 8, 1, 9, 2};
        real_min_ = 1;
        break;
      case 1:
        input_data_ = {10, -5, 7, 0, 15};
        real_min_ = -5;
        break;
      case 2:
        input_data_ = {42};
        real_min_ = 42;
        break;
      case 3:
        input_data_ = {0};
        real_min_ = 0;
        break;
      case 4:
        input_data_ = {-10, -140, -45, -24, -99};
        real_min_ = -140;
        break;
      default:
        input_data_ = {1, 2, 3};
        real_min_ = 1;
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return output_data == real_min_;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  int real_min_{};
};

namespace {

TEST_P(SinevAMinInVectorFuncTests, MatmulFromPic) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 6> kTestParam = {std::make_tuple(0, "mixed_positive"), std::make_tuple(1, "with_negatives"),
                                            std::make_tuple(2, "single_element"), std::make_tuple(3, "zero_only"),
                                            std::make_tuple(4, "all_negative")};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<SinevAMinInVectorMPI, InType>(kTestParam, PPC_SETTINGS_sinev_a_min_in_vector),
    ppc::util::AddFuncTask<SinevAMinInVectorSEQ, InType>(kTestParam, PPC_SETTINGS_sinev_a_min_in_vector));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = SinevAMinInVectorFuncTests::PrintFuncTestName<SinevAMinInVectorFuncTests>;

INSTANTIATE_TEST_SUITE_P(PicMatrixTests, SinevAMinInVectorFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace sinev_a_min_in_vector
