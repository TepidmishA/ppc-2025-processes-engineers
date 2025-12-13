#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace melnik_i_min_neigh_diff_vec {

using InType = std::vector<int>;
using OutType = std::tuple<int, int>;
using TestType = std::tuple<std::vector<int>, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace melnik_i_min_neigh_diff_vec
