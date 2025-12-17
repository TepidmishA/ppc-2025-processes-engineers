#pragma once

#include "pankov_a_string_word_count/common/include/common.hpp"
#include "task/include/task.hpp"

namespace pankov_a_string_word_count {

class PankovAStringWordCountMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit PankovAStringWordCountMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace pankov_a_string_word_count
