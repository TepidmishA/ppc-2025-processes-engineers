#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace ovchinnikov_m_bubble_sort {

using InType = std::vector<int>;
using OutType = std::vector<int>;
using TestType = std::tuple<std::vector<int>, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace ovchinnikov_m_bubble_sort
