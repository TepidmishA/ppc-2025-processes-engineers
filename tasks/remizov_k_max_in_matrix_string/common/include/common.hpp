#pragma once

#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace remizov_k_max_in_matrix_string {

using InType = std::vector<std::vector<int>>;
using OutType = std::vector<int>;
using TestType = std::tuple<std::vector<std::vector<int>>, std::vector<int>>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace remizov_k_max_in_matrix_string
