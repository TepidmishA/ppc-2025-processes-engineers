#pragma once

#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace ermakov_a_numb_viol_elem_vec {

using InType = std::vector<int>;
using OutType = int;
using TestType = std::tuple<std::vector<int>, int>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace ermakov_a_numb_viol_elem_vec
