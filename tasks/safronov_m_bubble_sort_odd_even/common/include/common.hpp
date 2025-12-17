#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace safronov_m_bubble_sort_odd_even {

using InType = std::vector<int>;
using OutType = std::vector<int>;
using TestType = std::tuple<std::string, std::vector<int>, std::vector<int>>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace safronov_m_bubble_sort_odd_even
