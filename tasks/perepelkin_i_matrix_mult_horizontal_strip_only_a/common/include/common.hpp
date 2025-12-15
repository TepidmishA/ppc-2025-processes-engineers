#pragma once

#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "task/include/task.hpp"

namespace perepelkin_i_matrix_mult_horizontal_strip_only_a {

using InType = std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>>;
using OutType = std::vector<std::vector<double>>;
using TestType = std::tuple<std::string, std::vector<std::vector<double>>, std::vector<std::vector<double>>,
                            std::vector<std::vector<double>>>;
using BaseTask = ppc::task::Task<InType, OutType>;
using DiffT = std::vector<double>::difference_type;

}  // namespace perepelkin_i_matrix_mult_horizontal_strip_only_a
