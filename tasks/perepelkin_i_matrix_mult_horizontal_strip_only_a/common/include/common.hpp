#pragma once

#include <string>
#include <utility>

#include "task/include/task.hpp"

namespace perepelkin_i_matrix_mult_horizontal_strip_only_a {

using InType = std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>>;
using OutType = std::vector<std::vector<double>>;
using TestType =
    std::tuple<std::vector<std::vector<double>>, std::vector<std::vector<double>>, std::vector<std::vector<double>>>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace perepelkin_i_matrix_mult_horizontal_strip_only_a
