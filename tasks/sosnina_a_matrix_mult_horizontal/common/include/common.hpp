#pragma once

#include <tuple>
#include <utility>
#include <vector>

#include "task/include/task.hpp"

namespace sosnina_a_matrix_mult_horizontal {

using InType = std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>>;
using OutType = std::vector<std::vector<double>>;
using TestType = std::tuple<int, std::vector<std::vector<double>>, std::vector<std::vector<double>>,
                            std::vector<std::vector<double>>>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace sosnina_a_matrix_mult_horizontal
