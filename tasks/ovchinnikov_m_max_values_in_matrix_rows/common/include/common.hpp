#pragma once

#include <cstddef>
#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace ovchinnikov_m_max_values_in_matrix_rows {

using InType = std::tuple<size_t, size_t, std::vector<int>>;
using OutType = std::vector<int>;
using TestType = std::tuple<size_t, size_t, std::vector<int>, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace ovchinnikov_m_max_values_in_matrix_rows
