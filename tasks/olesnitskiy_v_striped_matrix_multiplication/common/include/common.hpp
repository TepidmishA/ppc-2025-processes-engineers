#pragma once

#include <cstddef>
#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace olesnitskiy_v_striped_matrix_multiplication {
using InType = std::tuple<size_t, size_t, std::vector<double>, size_t, size_t, std::vector<double>>;
using OutType = std::tuple<size_t, size_t, std::vector<double>>;
using TestType = std::tuple<InType, OutType, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;
}  // namespace olesnitskiy_v_striped_matrix_multiplication
