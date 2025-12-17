#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace dilshodov_a_max_val_rows_matrix {

using InType = std::vector<std::vector<int>>;
using OutType = std::vector<int>;
using TestType = std::tuple<int, int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace dilshodov_a_max_val_rows_matrix
