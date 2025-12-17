#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "sosnina_a_matrix_mult_horizontal/common/include/common.hpp"
#include "sosnina_a_matrix_mult_horizontal/mpi/include/ops_mpi.hpp"
#include "sosnina_a_matrix_mult_horizontal/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace sosnina_a_matrix_mult_horizontal {

class SosninaAMatrixMultHorizontalFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param)) + "_matrix_mult";
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());

    matrixA_ = std::get<1>(params);
    matrixB_ = std::get<2>(params);
    expected_ = std::get<3>(params);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    // Проверяем размеры
    if (output_data.size() != expected_.size()) {
      return false;
    }
    if (!output_data.empty() && output_data[0].size() != expected_[0].size()) {
      return false;
    }

    // Используем достаточно маленький допуск
    const double tolerance = 1e-10;

    for (size_t i = 0; i < expected_.size(); i++) {
      for (size_t j = 0; j < expected_[i].size(); j++) {
        if (std::abs(output_data[i][j] - expected_[i][j]) > tolerance) {
          return false;
        }
      }
    }

    return true;
  }

  InType GetTestInputData() final {
    return std::make_pair(matrixA_, matrixB_);
  }

 private:
  std::vector<std::vector<double>> matrixA_;
  std::vector<std::vector<double>> matrixB_;
  std::vector<std::vector<double>> expected_;
};

namespace {

// Functional Tests
TEST_P(SosninaAMatrixMultHorizontalFuncTests, FunctionalTests) {
  ExecuteTest(GetParam());
}

// Coverage Tests
TEST_P(SosninaAMatrixMultHorizontalFuncTests, CoverageTests) {
  ExecuteTest(GetParam());
}

// Тестовые случаи: (id, matrixA, matrixB, expected)
const std::array<TestType, 22> kFunctionalTests = {
    // 1. Базовое умножение 2x2
    std::make_tuple(1, std::vector<std::vector<double>>{{1, 2}, {3, 4}},
                    std::vector<std::vector<double>>{{5, 6}, {7, 8}},
                    std::vector<std::vector<double>>{{19, 22}, {43, 50}}),

    // 2. Единичная матрица
    std::make_tuple(2, std::vector<std::vector<double>>{{1, 0}, {0, 1}},
                    std::vector<std::vector<double>>{{1, 2}, {3, 4}}, std::vector<std::vector<double>>{{1, 2}, {3, 4}}),

    // 3. Матрица из единиц
    std::make_tuple(3, std::vector<std::vector<double>>{{1, 1}, {1, 1}},
                    std::vector<std::vector<double>>{{1, 1}, {1, 1}}, std::vector<std::vector<double>>{{2, 2}, {2, 2}}),

    // 4. Диагональные матрицы
    std::make_tuple(4, std::vector<std::vector<double>>{{2, 0}, {0, 2}},
                    std::vector<std::vector<double>>{{3, 0}, {0, 3}}, std::vector<std::vector<double>>{{6, 0}, {0, 6}}),

    // 5. Вектор-строка на вектор-столбец
    std::make_tuple(5, std::vector<std::vector<double>>{{1, 2, 3}}, std::vector<std::vector<double>>{{4}, {5}, {6}},
                    std::vector<std::vector<double>>{{32}}),

    // 6. Вектор-столбец на вектор-строку
    std::make_tuple(6, std::vector<std::vector<double>>{{1}, {2}, {3}}, std::vector<std::vector<double>>{{4, 5, 6}},
                    std::vector<std::vector<double>>{{4, 5, 6}, {8, 10, 12}, {12, 15, 18}}),

    // 7. Неквадратные матрицы
    std::make_tuple(7, std::vector<std::vector<double>>{{1, 2}, {3, 4}},
                    std::vector<std::vector<double>>{{5, 6}, {7, 8}},
                    std::vector<std::vector<double>>{{19, 22}, {43, 50}}),

    // 8. 1x1 матрицы
    std::make_tuple(8, std::vector<std::vector<double>>{{1}}, std::vector<std::vector<double>>{{1}},
                    std::vector<std::vector<double>>{{1}}),

    // 9. Нулевая матрица
    std::make_tuple(9, std::vector<std::vector<double>>{{0, 0}, {0, 0}},
                    std::vector<std::vector<double>>{{1, 2}, {3, 4}}, std::vector<std::vector<double>>{{0, 0}, {0, 0}}),

    // 10. Скалярное умножение
    std::make_tuple(10, std::vector<std::vector<double>>{{2}}, std::vector<std::vector<double>>{{3}},
                    std::vector<std::vector<double>>{{6}}),

    // 11. 2x3 на 3x2
    std::make_tuple(11, std::vector<std::vector<double>>{{1, 2, 3}, {4, 5, 6}},
                    std::vector<std::vector<double>>{{7, 8}, {9, 10}, {11, 12}},
                    std::vector<std::vector<double>>{{58, 64}, {139, 154}}),

    // 12. Единичная матрица 3x3
    std::make_tuple(12, std::vector<std::vector<double>>{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},
                    std::vector<std::vector<double>>{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}},
                    std::vector<std::vector<double>>{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}),

    // 13. Дробные числа
    std::make_tuple(13, std::vector<std::vector<double>>{{0.5, 0.5}, {0.5, 0.5}},
                    std::vector<std::vector<double>>{{2, 4}, {6, 8}}, std::vector<std::vector<double>>{{4, 6}, {4, 6}}),

    // 14. 3x2 на 2x3
    std::make_tuple(14, std::vector<std::vector<double>>{{1, 1}, {1, 1}, {1, 1}},
                    std::vector<std::vector<double>>{{1, 1, 1}, {1, 1, 1}},
                    std::vector<std::vector<double>>{{2, 2, 2}, {2, 2, 2}, {2, 2, 2}}),

    // 15. Обратное умножение
    std::make_tuple(15, std::vector<std::vector<double>>{{2, 4}, {6, 8}},
                    std::vector<std::vector<double>>{{1, 0}, {0, 1}}, std::vector<std::vector<double>>{{2, 4}, {6, 8}}),

    // 16. 3x2 на 2x3
    std::make_tuple(16, std::vector<std::vector<double>>{{1, 2}, {3, 4}, {5, 6}},
                    std::vector<std::vector<double>>{{7, 8, 9}, {10, 11, 12}},
                    std::vector<std::vector<double>>{{27, 30, 33}, {61, 68, 75}, {95, 106, 117}}),

    // 17. Дробные числа 2
    std::make_tuple(17, std::vector<std::vector<double>>{{0.1, 0.2}, {0.3, 0.4}},
                    std::vector<std::vector<double>>{{5, 6}, {7, 8}},
                    std::vector<std::vector<double>>{{1.9, 2.2}, {4.3, 5.0}}),

    // 18. Матрица из двоек
    std::make_tuple(18, std::vector<std::vector<double>>{{1, 1}, {1, 1}},
                    std::vector<std::vector<double>>{{2, 2}, {2, 2}}, std::vector<std::vector<double>>{{4, 4}, {4, 4}}),

    // 19. Скаляр 3x4
    std::make_tuple(19, std::vector<std::vector<double>>{{3}}, std::vector<std::vector<double>>{{4}},
                    std::vector<std::vector<double>>{{12}}),

    // 20. Частично нулевая матрица
    std::make_tuple(20, std::vector<std::vector<double>>{{1, 0}, {0, 0}},
                    std::vector<std::vector<double>>{{0, 1}, {0, 0}}, std::vector<std::vector<double>>{{0, 1}, {0, 0}}),

    // 21. Обратная единичная
    std::make_tuple(21, std::vector<std::vector<double>>{{2, 3}, {4, 5}},
                    std::vector<std::vector<double>>{{1, 0}, {0, 1}}, std::vector<std::vector<double>>{{2, 3}, {4, 5}}),

    // 22. Умножение на нулевую матрицу
    std::make_tuple(22, std::vector<std::vector<double>>{{1, 2}, {3, 4}},
                    std::vector<std::vector<double>>{{0, 0}, {0, 0}},
                    std::vector<std::vector<double>>{{0, 0}, {0, 0}})};

const std::array<TestType, 10> kCoverageTests = {
    std::make_tuple(23, std::vector<std::vector<double>>{{1}}, std::vector<std::vector<double>>{{1}},
                    std::vector<std::vector<double>>{{1}}),

    std::make_tuple(24, std::vector<std::vector<double>>{{0}}, std::vector<std::vector<double>>{{0}},
                    std::vector<std::vector<double>>{{0}}),

    std::make_tuple(25, std::vector<std::vector<double>>{{1, 2}, {3, 4}},
                    std::vector<std::vector<double>>{{1, 0}, {0, 1}}, std::vector<std::vector<double>>{{1, 2}, {3, 4}}),

    std::make_tuple(26, std::vector<std::vector<double>>{{1, 1}, {1, 1}},
                    std::vector<std::vector<double>>{{2, 2}, {2, 2}}, std::vector<std::vector<double>>{{4, 4}, {4, 4}}),

    std::make_tuple(27, std::vector<std::vector<double>>{{0.5}}, std::vector<std::vector<double>>{{2}},
                    std::vector<std::vector<double>>{{1}}),

    std::make_tuple(28, std::vector<std::vector<double>>{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},
                    std::vector<std::vector<double>>{{5, 6, 7}, {8, 9, 10}, {11, 12, 13}},
                    std::vector<std::vector<double>>{{5, 6, 7}, {8, 9, 10}, {11, 12, 13}}),

    std::make_tuple(29, std::vector<std::vector<double>>{{2, 4}, {6, 8}},
                    std::vector<std::vector<double>>{{0.5, 0}, {0, 0.5}},
                    std::vector<std::vector<double>>{{1, 2}, {3, 4}}),

    // 30. Исправленная версия: 2x3 на 3x2 (как тест 11)
    std::make_tuple(30, std::vector<std::vector<double>>{{1, 2, 3}, {4, 5, 6}},   // 2x3
                    std::vector<std::vector<double>>{{7, 8}, {9, 10}, {11, 12}},  // 3x2
                    std::vector<std::vector<double>>{{58, 64}, {139, 154}}),      // 2x2

    std::make_tuple(31, std::vector<std::vector<double>>{{1}, {2}, {3}}, std::vector<std::vector<double>>{{4, 5, 6}},
                    std::vector<std::vector<double>>{{4, 5, 6}, {8, 10, 12}, {12, 15, 18}}),

    std::make_tuple(32, std::vector<std::vector<double>>{{1, 2, 3}}, std::vector<std::vector<double>>{{4}, {5}, {6}},
                    std::vector<std::vector<double>>{{32}})};

const auto kFunctionalTasksList =
    std::tuple_cat(ppc::util::AddFuncTask<sosnina_a_matrix_mult_horizontal::SosninaAMatrixMultHorizontalMPI, InType>(
                       kFunctionalTests, PPC_SETTINGS_sosnina_a_matrix_mult_horizontal),
                   ppc::util::AddFuncTask<sosnina_a_matrix_mult_horizontal::SosninaAMatrixMultHorizontalSEQ, InType>(
                       kFunctionalTests, PPC_SETTINGS_sosnina_a_matrix_mult_horizontal));

const auto kCoverageTasksList =
    std::tuple_cat(ppc::util::AddFuncTask<sosnina_a_matrix_mult_horizontal::SosninaAMatrixMultHorizontalMPI, InType>(
                       kCoverageTests, PPC_SETTINGS_sosnina_a_matrix_mult_horizontal),
                   ppc::util::AddFuncTask<sosnina_a_matrix_mult_horizontal::SosninaAMatrixMultHorizontalSEQ, InType>(
                       kCoverageTests, PPC_SETTINGS_sosnina_a_matrix_mult_horizontal));

inline const auto kFunctionalGtestValues = ppc::util::ExpandToValues(kFunctionalTasksList);
inline const auto kCoverageGtestValues = ppc::util::ExpandToValues(kCoverageTasksList);

inline const auto kPerfTestName =
    SosninaAMatrixMultHorizontalFuncTests::PrintFuncTestName<SosninaAMatrixMultHorizontalFuncTests>;

INSTANTIATE_TEST_SUITE_P(Functional, SosninaAMatrixMultHorizontalFuncTests, kFunctionalGtestValues, kPerfTestName);
INSTANTIATE_TEST_SUITE_P(Coverage, SosninaAMatrixMultHorizontalFuncTests, kCoverageGtestValues, kPerfTestName);

}  // namespace

}  // namespace sosnina_a_matrix_mult_horizontal
