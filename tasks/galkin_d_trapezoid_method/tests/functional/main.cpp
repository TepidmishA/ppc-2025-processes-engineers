#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <memory>
#include <numbers>
#include <string>
#include <tuple>

#include "galkin_d_trapezoid_method/common/include/common.hpp"
#include "galkin_d_trapezoid_method/mpi/include/ops_mpi.hpp"
#include "galkin_d_trapezoid_method/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

constexpr double kPi = std::numbers::pi;

namespace galkin_d_trapezoid_method {

class GalkinDTrapezoidFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param)) + "_" + std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    const int case_id = std::get<0>(params);
    switch (case_id) {
      case 0: {
        input_data_ = {.a = 0.0, .b = 1.0, .n = 1000, .func_id = static_cast<int>(FunctionId::kLinear)};
        break;
      }
      case 1: {
        input_data_ = {.a = 0.0, .b = 2.0, .n = 2000, .func_id = static_cast<int>(FunctionId::kQuadratic)};
        break;
      }
      case 2: {
        input_data_ = {.a = 0.0, .b = kPi, .n = 3000, .func_id = static_cast<int>(FunctionId::kSin)};
        break;
      }
      case 3: {
        input_data_ = {.a = -1.0, .b = 1.0, .n = 1500, .func_id = static_cast<int>(FunctionId::kLinear)};
        break;
      }
      default: {
        input_data_ = {.a = 0.0, .b = 1.0, .n = 1000, .func_id = static_cast<int>(FunctionId::kLinear)};
        break;
      }
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    const double exact = GetExactIntegral(input_data_);
    const double eps = 1e-4;

    return std::fabs(output_data - exact) < eps;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_{.a = 0.0, .b = 1.0, .n = 10, .func_id = static_cast<int>(FunctionId::kLinear)};
};

namespace {

TEST_P(GalkinDTrapezoidFuncTests, ComputesIntegralWithReasonableAccuracy) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 4> kFunctionalParams = {std::make_tuple(0, "linear_0_1"),
                                                   std::make_tuple(1, "quadratic_0_2"), std::make_tuple(2, "sin_0_pi"),
                                                   std::make_tuple(3, "linear_m1_1")};

const auto kTaskMatrix = std::tuple_cat(ppc::util::AddFuncTask<GalkinDTrapezoidMethodMPI, InType>(
                                            kFunctionalParams, PPC_SETTINGS_galkin_d_trapezoid_method),
                                        ppc::util::AddFuncTask<GalkinDTrapezoidMethodSEQ, InType>(
                                            kFunctionalParams, PPC_SETTINGS_galkin_d_trapezoid_method));

const auto kParameterizedValues = ppc::util::ExpandToValues(kTaskMatrix);

const auto kFunctionalTestName = GalkinDTrapezoidFuncTests::PrintFuncTestName<GalkinDTrapezoidFuncTests>;

INSTANTIATE_TEST_SUITE_P(TrapezoidIntegralSuite, GalkinDTrapezoidFuncTests, kParameterizedValues, kFunctionalTestName);

template <typename TaskType>
void ExpectFullPipelineSuccess(const InType &in, double eps = 1e-4) {
  auto task = std::make_shared<TaskType>(in);
  ASSERT_TRUE(task->Validation());
  ASSERT_TRUE(task->PreProcessing());
  ASSERT_TRUE(task->Run());
  ASSERT_TRUE(task->PostProcessing());

  const double exact = GetExactIntegral(in);
  ASSERT_NEAR(task->GetOutput(), exact, eps);
}

TEST(GalkinDTrapezoidStandalone, SeqPipelineStandardCases) {
  const std::array<InType, 3> k_inputs = {
      InType{.a = 0.0, .b = 1.0, .n = 1000, .func_id = static_cast<int>(FunctionId::kLinear)},
      InType{.a = 0.0, .b = 2.0, .n = 2000, .func_id = static_cast<int>(FunctionId::kQuadratic)},
      InType{.a = 0.0, .b = kPi, .n = 4000, .func_id = static_cast<int>(FunctionId::kSin)},
  };

  for (const auto &in : k_inputs) {
    ExpectFullPipelineSuccess<GalkinDTrapezoidMethodSEQ>(in);
  }
}

TEST(GalkinDTrapezoidStandalone, MpiPipelineStandardCases) {
  if (!ppc::util::IsUnderMpirun()) {
    GTEST_SKIP();
  }
  const std::array<InType, 3> k_inputs = {
      InType{.a = 0.0, .b = 1.0, .n = 1000, .func_id = static_cast<int>(FunctionId::kLinear)},
      InType{.a = 0.0, .b = 2.0, .n = 2000, .func_id = static_cast<int>(FunctionId::kQuadratic)},
      InType{.a = 0.0, .b = kPi, .n = 4000, .func_id = static_cast<int>(FunctionId::kSin)},
  };

  for (const auto &in : k_inputs) {
    ExpectFullPipelineSuccess<GalkinDTrapezoidMethodMPI>(in);
  }
}

TEST(GalkinDTrapezoidValidation, RejectsNonPositiveNSeq) {
  InType in{.a = 0.0, .b = 1.0, .n = 0, .func_id = static_cast<int>(FunctionId::kLinear)};
  GalkinDTrapezoidMethodSEQ task(in);
  EXPECT_FALSE(task.Validation());
  task.PreProcessing();
  task.Run();
  task.PostProcessing();
}

TEST(GalkinDTrapezoidValidation, RejectsNonPositiveNMpi) {
  if (!ppc::util::IsUnderMpirun()) {
    GTEST_SKIP();
  }
  InType in{.a = 0.0, .b = 1.0, .n = 0, .func_id = static_cast<int>(FunctionId::kLinear)};
  GalkinDTrapezoidMethodMPI task(in);
  EXPECT_FALSE(task.Validation());
  task.PreProcessing();
  task.Run();
  task.PostProcessing();
}

TEST(GalkinDTrapezoidValidation, RejectsInvalidIntervalSeq) {
  InType in{.a = 1.0, .b = 0.0, .n = 100, .func_id = static_cast<int>(FunctionId::kLinear)};
  GalkinDTrapezoidMethodSEQ task(in);
  EXPECT_FALSE(task.Validation());
}

TEST(GalkinDTrapezoidValidation, RejectsInvalidIntervalMpi) {
  if (!ppc::util::IsUnderMpirun()) {
    GTEST_SKIP();
  }
  InType in{.a = 1.0, .b = 0.0, .n = 100, .func_id = static_cast<int>(FunctionId::kLinear)};
  GalkinDTrapezoidMethodMPI task(in);
  EXPECT_FALSE(task.Validation());
}

TEST(GalkinDTrapezoidValidation, AcceptsValidInputSeq) {
  InType in{.a = 0.0, .b = 1.0, .n = 1000, .func_id = static_cast<int>(FunctionId::kLinear)};
  GalkinDTrapezoidMethodSEQ task(in);
  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());
  EXPECT_TRUE(task.Run());
  EXPECT_TRUE(task.PostProcessing());

  const double exact = GetExactIntegral(in);
  EXPECT_NEAR(task.GetOutput(), exact, 1e-4);
}

TEST(GalkinDTrapezoidValidation, AcceptsValidInputMpi) {
  if (!ppc::util::IsUnderMpirun()) {
    GTEST_SKIP();
  }
  InType in{.a = 0.0, .b = 1.0, .n = 1000, .func_id = static_cast<int>(FunctionId::kLinear)};
  GalkinDTrapezoidMethodMPI task(in);
  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());
  EXPECT_TRUE(task.Run());
  EXPECT_TRUE(task.PostProcessing());

  const double exact = GetExactIntegral(in);
  EXPECT_NEAR(task.GetOutput(), exact, 1e-4);
}

template <typename TaskType>
void RunTaskTwice(TaskType &task, const InType &first, const InType &second, double eps = 1e-4) {
  task.GetInput() = first;
  task.GetOutput() = 0.0;
  ASSERT_TRUE(task.Validation());
  ASSERT_TRUE(task.PreProcessing());
  ASSERT_TRUE(task.Run());
  ASSERT_TRUE(task.PostProcessing());
  ASSERT_NEAR(task.GetOutput(), GetExactIntegral(first), eps);

  task.GetInput() = second;
  task.GetOutput() = 0.0;
  ASSERT_TRUE(task.Validation());
  ASSERT_TRUE(task.PreProcessing());
  ASSERT_TRUE(task.Run());
  ASSERT_TRUE(task.PostProcessing());
  ASSERT_NEAR(task.GetOutput(), GetExactIntegral(second), eps);
}

TEST(GalkinDTrapezoidPipeline, SeqTaskCanBeReusedAcrossRuns) {
  InType first{.a = 0.0, .b = 1.0, .n = 1000, .func_id = static_cast<int>(FunctionId::kLinear)};
  InType second{.a = 0.0, .b = kPi, .n = 3000, .func_id = static_cast<int>(FunctionId::kSin)};

  GalkinDTrapezoidMethodSEQ task(first);
  RunTaskTwice(task, first, second);
}

TEST(GalkinDTrapezoidPipeline, MpiTaskCanBeReusedAcrossRuns) {
  if (!ppc::util::IsUnderMpirun()) {
    GTEST_SKIP();
  }

  InType first{.a = 0.0, .b = 1.0, .n = 1000, .func_id = static_cast<int>(FunctionId::kLinear)};
  InType second{.a = 0.0, .b = kPi, .n = 3000, .func_id = static_cast<int>(FunctionId::kSin)};

  GalkinDTrapezoidMethodMPI task(first);
  RunTaskTwice(task, first, second);
}

TEST(GalkinDTrapezoidCommon, FunctionReturnsZeroOnUnknownId) {
  const double x = 1.234;
  const double value = Function(x, 42);
  EXPECT_DOUBLE_EQ(value, 0.0);
}

TEST(GalkinDTrapezoidCommon, GetExactIntegralReturnsZeroOnUnknownId) {
  InType in{.a = 0.0, .b = 1.0, .n = 10, .func_id = 42};
  const double exact = GetExactIntegral(in);
  EXPECT_DOUBLE_EQ(exact, 0.0);
}

}  // namespace

}  // namespace galkin_d_trapezoid_method
