#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace afanasyev_a_elem_vec_avg {

using T = int;
using InType = std::vector<T>;
using OutType = double;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace afanasyev_a_elem_vec_avg
