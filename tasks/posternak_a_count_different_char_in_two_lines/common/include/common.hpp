#pragma once

#include <string>
#include <tuple>
#include <utility>

#include "task/include/task.hpp"

namespace posternak_a_count_different_char_in_two_lines {

using InType = std::pair<std::string, std::string>;
using OutType = int;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace posternak_a_count_different_char_in_two_lines
