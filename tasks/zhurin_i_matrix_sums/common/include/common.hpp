#pragma once

#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace zhurin_i_matrix_sums {

constexpr double kEpsilon = 1e-10;
using InType = std::tuple<uint32_t, uint32_t, std::vector<double>>;
using OutType = double;
using TestType = std::string;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace zhurin_i_matrix_sums
