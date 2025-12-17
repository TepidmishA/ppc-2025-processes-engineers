#include <gtest/gtest.h>
#include <mpi.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <exception>
#include <limits>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

#include "baranov_a_custom_allreduce/common/include/common.hpp"
#include "baranov_a_custom_allreduce/mpi/include/ops_mpi.hpp"
#include "baranov_a_custom_allreduce/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"

namespace baranov_a_custom_allreduce {

class BaranovACustomAllreduceFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::get<1>(test_param);
  }

 protected:
  BaranovACustomAllreduceFuncTests() : data_type_(MPI_DATATYPE_NULL) {}

  void SetUp() override {
    auto param = GetParam();
    TestType test_param = std::get<2>(param);
    int test_case = std::get<0>(test_param);
    std::string task_name = std::get<1>(param);
    is_mpi_test_ = (task_name.find("mpi") != std::string::npos);

    switch (test_case) {
      case 1: {
        std::vector<double> input = {1.0, 2.0, 3.0, 4.0, 5.0};
        input_data_ = InTypeVariant{input};
        expected_input_double_ = input;
        data_type_ = MPI_DOUBLE;
      } break;
      case 2: {
        std::vector<double> input = {1.0, -2.0, 3.0, -4.0, 5.0};
        input_data_ = InTypeVariant{input};
        expected_input_double_ = input;
        data_type_ = MPI_DOUBLE;
      } break;
      case 3: {
        std::vector<double> input = {-1.0, -2.0, -3.0, -4.0, -5.0};
        input_data_ = InTypeVariant{input};
        expected_input_double_ = input;
        data_type_ = MPI_DOUBLE;
      } break;
      case 4: {
        std::vector<double> input = {1000.0, 2000.0, 3000.0};
        input_data_ = InTypeVariant{input};
        expected_input_double_ = input;
        data_type_ = MPI_DOUBLE;
      } break;
      case 5: {
        std::vector<double> input = {1.5, 2.5, 3.5};
        input_data_ = InTypeVariant{input};
        expected_input_double_ = input;
        data_type_ = MPI_DOUBLE;
      } break;
      case 6: {
        std::vector<double> input = {};
        input_data_ = InTypeVariant{input};
        expected_input_double_ = input;
        data_type_ = MPI_DOUBLE;
      } break;
      case 7: {
        std::vector<double> input = {42.0};
        input_data_ = InTypeVariant{input};
        expected_input_double_ = input;
        data_type_ = MPI_DOUBLE;
      } break;
      case 8: {
        std::vector<double> input = {0.0, 0.0, 0.0, 0.0};
        input_data_ = InTypeVariant{input};
        expected_input_double_ = input;
        data_type_ = MPI_DOUBLE;
      } break;
      case 9: {
        std::vector<int> input = {1, 2, 3, 4};
        input_data_ = InTypeVariant{input};
        expected_input_int_ = input;
        data_type_ = MPI_INT;
      } break;
      case 10: {
        std::vector<float> input = {1.1F, 2.2F, 3.3F};
        input_data_ = InTypeVariant{input};
        expected_input_float_ = input;
        data_type_ = MPI_FLOAT;
      } break;
      case 12: {
        std::vector<double> input = {1.0, std::numeric_limits<double>::quiet_NaN(), 3.0,
                                     std::numeric_limits<double>::signaling_NaN()};
        input_data_ = InTypeVariant{input};
        expected_input_double_ = input;
        data_type_ = MPI_DOUBLE;
      } break;

      case 13: {
        std::vector<double> input = {1.0, std::numeric_limits<double>::infinity(),
                                     -std::numeric_limits<double>::infinity(), 4.0};
        input_data_ = InTypeVariant{input};
        expected_input_double_ = input;
        data_type_ = MPI_DOUBLE;
      } break;

      case 14: {
        const int size = 1000;
        std::vector<double> input(size);
        std::vector<int> input_int(size);
        std::vector<float> input_float(size);
        if (is_mpi_test_) {
          if (task_name.find("int") != std::string::npos) {
            for (int i = 0; i < size; i++) {
              input_int[i] = i * 2;
            }
            input_data_ = InTypeVariant{input_int};
            expected_input_int_ = input_int;
            data_type_ = MPI_INT;
          } else if (task_name.find("float") != std::string::npos) {
            for (int i = 0; i < size; i++) {
              input_float[i] = static_cast<float>(i) * 1.5F;
            }
            input_data_ = InTypeVariant{input_float};
            expected_input_float_ = input_float;
            data_type_ = MPI_FLOAT;
          } else {
            for (int i = 0; i < size; i++) {
              input[i] = static_cast<double>(i) * 1.7;
            }
            input_data_ = InTypeVariant{input};
            expected_input_double_ = input;
            data_type_ = MPI_DOUBLE;
          }
        } else {
          for (int i = 0; i < size; i++) {
            input[i] = static_cast<double>(i) * 1.7;
          }
          input_data_ = InTypeVariant{input};
          expected_input_double_ = input;
          data_type_ = MPI_DOUBLE;
        }
      } break;
      default: {
        std::vector<double> input = {1.0, 2.0, 3.0};
        input_data_ = InTypeVariant{input};
        expected_input_double_ = input;
        data_type_ = MPI_DOUBLE;
      } break;
    }
  }

 private:
  bool CheckMpiIntOutput(const std::vector<int> &output_vec, int world_size) {
    auto expected_vec = expected_input_int_;
    if (output_vec.size() != expected_vec.size()) {
      return false;
    }
    for (std::size_t i = 0; i < output_vec.size(); ++i) {
      int expected = expected_vec[i] * world_size;
      if (output_vec[i] != expected) {
        return false;
      }
    }
    return true;
  }

  bool CheckMpiFloatOutput(const std::vector<float> &output_vec, int world_size) {
    auto expected_vec = expected_input_float_;
    if (output_vec.size() != expected_vec.size()) {
      return false;
    }
    const float epsilon = 1e-5F;
    for (std::size_t i = 0; i < output_vec.size(); ++i) {
      float expected = expected_vec[i] * static_cast<float>(world_size);
      bool output_is_nan = std::isnan(output_vec[i]);
      bool expected_is_nan = std::isnan(expected);
      if (output_is_nan != expected_is_nan) {
        return false;
      }
      if (!output_is_nan && std::fabs(output_vec[i] - expected) > epsilon) {
        return false;
      }
    }
    return true;
  }

  static bool CompareDoubleWithNanInf(double output, double expected, double epsilon) {
    bool output_is_nan = std::isnan(output);
    bool expected_is_nan = std::isnan(expected);
    if (output_is_nan != expected_is_nan) {
      return false;
    }
    bool output_is_inf = std::isinf(output);
    bool expected_is_inf = std::isinf(expected);
    if (output_is_inf != expected_is_inf) {
      return false;
    }
    if (output_is_inf && expected_is_inf) {
      return std::signbit(output) == std::signbit(expected);
    }
    if (!output_is_nan && !output_is_inf) {
      return std::fabs(output - expected) <= epsilon;
    }
    return true;
  }

  bool CheckMpiDoubleOutput(const std::vector<double> &output_vec, int world_size) {
    auto expected_vec = expected_input_double_;
    if (output_vec.size() != expected_vec.size()) {
      return false;
    }
    const double epsilon = 1e-10;
    for (std::size_t i = 0; i < output_vec.size(); ++i) {
      double expected = expected_vec[i] * static_cast<double>(world_size);
      if (!CompareDoubleWithNanInf(output_vec[i], expected, epsilon)) {
        return false;
      }
    }
    return true;
  }

  bool CheckSeqDoubleOutput(const std::vector<double> &output_vec) {
    auto input_vec = std::get<std::vector<double>>(input_data_);
    if (output_vec.size() != input_vec.size()) {
      return false;
    }
    for (std::size_t i = 0; i < output_vec.size(); ++i) {
      if (!CompareDoubleWithNanInf(output_vec[i], input_vec[i], 0.0)) {
        return false;
      }
    }
    return true;
  }

 public:
  bool CheckTestOutputData(OutType &output_data) final {
    try {
      if (is_mpi_test_) {
        int mpi_initialized = 0;
        MPI_Initialized(&mpi_initialized);
        if (mpi_initialized == 0) {
          return true;
        }

        int world_size = 1;
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);

        if (std::holds_alternative<std::vector<int>>(output_data)) {
          return CheckMpiIntOutput(std::get<std::vector<int>>(output_data), world_size);
        }
        if (std::holds_alternative<std::vector<float>>(output_data)) {
          return CheckMpiFloatOutput(std::get<std::vector<float>>(output_data), world_size);
        }
        if (std::holds_alternative<std::vector<double>>(output_data)) {
          return CheckMpiDoubleOutput(std::get<std::vector<double>>(output_data), world_size);
        }
        return false;
      }

      if (std::holds_alternative<std::vector<double>>(output_data)) {
        return CheckSeqDoubleOutput(std::get<std::vector<double>>(output_data));
      }

      return output_data == input_data_;

    } catch (const std::exception &) {
      return false;
    }
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  std::vector<double> expected_input_double_;
  std::vector<int> expected_input_int_;
  std::vector<float> expected_input_float_;
  MPI_Datatype data_type_;
  bool is_mpi_test_ = false;
};

namespace {

TEST_P(BaranovACustomAllreduceFuncTests, AllreduceTest) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 14> kTestParam = {
    std::make_tuple(1, "positive_doubles"),   std::make_tuple(2, "mixed_doubles"),
    std::make_tuple(3, "negative_doubles"),   std::make_tuple(4, "large_doubles"),
    std::make_tuple(5, "fractional_doubles"), std::make_tuple(6, "empty_vector"),
    std::make_tuple(7, "single_element"),     std::make_tuple(8, "zeros"),
    std::make_tuple(9, "integers"),           std::make_tuple(10, "floats"),
    std::make_tuple(11, "float_with_suffix"), std::make_tuple(12, "nan_test"),
    std::make_tuple(13, "inf_test"),          std::make_tuple(14, "large_count_test"),
};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<BaranovACustomAllreduceMPI, InType>(kTestParam, PPC_SETTINGS_baranov_a_custom_allreduce),
    ppc::util::AddFuncTask<BaranovACustomAllreduceSEQ, InType>(kTestParam, PPC_SETTINGS_baranov_a_custom_allreduce));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = BaranovACustomAllreduceFuncTests::PrintFuncTestName<BaranovACustomAllreduceFuncTests>;

INSTANTIATE_TEST_SUITE_P(CustomAllreduceFuncTests, BaranovACustomAllreduceFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace baranov_a_custom_allreduce
