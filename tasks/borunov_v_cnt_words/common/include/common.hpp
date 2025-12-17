#pragma once

#include <cstddef>
#include <string>
#include <tuple>

#include "task/include/task.hpp"

namespace borunov_v_cnt_words {

using InType = std::string;
using OutType = size_t;
using TestType = std::tuple<InType, OutType>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace borunov_v_cnt_words
