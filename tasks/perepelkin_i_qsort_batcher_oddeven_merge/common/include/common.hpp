#pragma once

#include <string>
#include <utility>
#include <vector>

#include "task/include/task.hpp"

namespace perepelkin_i_qsort_batcher_oddeven_merge {

using InType = std::vector<double>;
using OutType = std::vector<double>;
using TestType = std::pair<std::string, std::vector<double>>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace perepelkin_i_qsort_batcher_oddeven_merge
