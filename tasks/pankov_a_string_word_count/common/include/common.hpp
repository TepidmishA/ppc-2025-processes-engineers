#pragma once

#include <string>
#include <tuple>

#include "task/include/task.hpp"

namespace pankov_a_string_word_count {

using InType = std::string;
using OutType = int;
using TestType = std::tuple<InType, OutType>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace pankov_a_string_word_count
