#pragma once

#include <string>
#include <tuple>
#include <utility>

#include "task/include/task.hpp"

namespace marin_l_cnt_mismat_chrt_in_two_str {

using InType = std::pair<std::string, std::string>;

using OutType = int;

using TestType = std::tuple<int, std::string>;

using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace marin_l_cnt_mismat_chrt_in_two_str
