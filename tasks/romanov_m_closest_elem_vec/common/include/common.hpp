#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace romanov_m_closest_elem_vec {

using InType = std::vector<int>;
using OutType = std::tuple<int, int>;
using TestType = std::tuple<std::vector<int>, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace romanov_m_closest_elem_vec
