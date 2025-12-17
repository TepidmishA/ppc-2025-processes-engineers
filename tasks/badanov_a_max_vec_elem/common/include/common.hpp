#pragma once

#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace badanov_a_max_vec_elem {

using InType = std::vector<int>;
using OutType = int;
using TestType = std::tuple<int, std::vector<int>>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace badanov_a_max_vec_elem
