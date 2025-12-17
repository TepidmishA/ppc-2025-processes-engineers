#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace zaharov_g_matrix_col_sum {

using InType = std::vector<std::vector<double>>;
using OutType = std::vector<double>;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace zaharov_g_matrix_col_sum
