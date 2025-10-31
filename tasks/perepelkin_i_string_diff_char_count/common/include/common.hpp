#pragma once

#include <string>
#include <tuple>

#include "task/include/task.hpp"

namespace perepelkin_i_string_diff_char_count {

using InType = int;
using OutType = int;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace perepelkin_i_string_diff_char_count
