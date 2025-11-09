#pragma once

#include <string>
#include <utility>

#include "task/include/task.hpp"

namespace perepelkin_i_string_diff_char_count {

using InType = std::pair<std::string, std::string>;
using OutType = int;
using TestType = std::pair<std::string, int>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace perepelkin_i_string_diff_char_count
