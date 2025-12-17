# Отчет по лабораторной работе №1
## Работу выполнил студент группы 3823Б1ПР1, Журин Иван Эдуардович
## Вариант № 10. Сумма элементов матрицы.

**Преподаватель:** Сысоев Александр Владимирович, лектор, доцент кафедры высокопроизводительных вычислений и системного программирования

---

## Введение

**Цель работы:** исследование методов параллельного программирования с использованием технологии MPI (Message Passing Interface) на примере задачи вычисления суммы элементов матрицы.

**Актуальность:** с ростом объемов обрабатываемых данных последовательные алгоритмы становятся недостаточно эффективными. Применение параллельных технологий позволяет обрабатывать большие массивы данных без значительного увеличения времени вычислений.

В рамках работы требовалось реализовать два варианта алгоритма:
- **Последовательный (SEQ)** — обработка данных в одном процессе
- **Параллельный (MPI)** — распределение работы между несколькими процессами с последующим объединением результатов

---

## Постановка задачи

Разработать программу, вычисляющую сумму всех элементов матрицы. Требуется реализовать:

- **Последовательный алгоритм (SEQ)** — вычисления в одном процессе
- **Параллельный алгоритм (MPI)** — распределение элементов матрицы между процессами и объединение частичных сумм

---

## Описание алгоритма

### Последовательный алгоритм (SEQ)

1. Инициализация переменной `sum` для накопления суммы
2. Последовательный обход всех элементов матрицы
3. Добавление каждого элемента к общей сумме
4. Возврат итогового значения

### Параллельный алгоритм (MPI)

1. **Распределение данных:** элементы матрицы распределяются между процессами
2. **Балансировка нагрузки:** базовое количество элементов на процесс вычисляется как `size/wsize`, остаток распределяется по одному элементу на первые `size%wsize` процессов
3. **Локальные вычисления:** каждый процесс вычисляет сумму своего блока данных
4. **Глобальная редукция:** объединение частичных сумм всех процессов

---

## Описание схемы параллельного алгоритма

1. **Инициализация** — получение параметров MPI
2. **Распространение размеров матрицы** — широковещательная рассылка размеров
3. **Расчет распределения данных** — определение counts и displs для Scatterv
4. **Подготовка данных** — процесс 0 инициализирует матрицу
5. **Распределение данных** — Scatterv для рассылки блоков матрицы
6. **Локальные вычисления** — подсчет частичных сумм
7. **Глобальная редукция** — Allreduce для получения общей суммы
8. **Сохранение результата** — вывод итогового значения

---

## Описание программной реализации

### Параллельная реализация (MPI)

**Архитектура коммуникации:**
- Равноправные процессы в коммуникаторе MPI_COMM_WORLD
- Коллективные операции вместо точечных
- Децентрализованные вычисления

### Фазы выполнения MPI-алгоритма

#### ValidationImpl()
- Локальная проверка корректности входных данных
- Подтверждение соответствия размеров матрицы

#### PreProcessingImpl()
- Инициализация выходного значения
- Подготовка исходных данных на процессе 0

#### RunImpl()
- **MPI_Bcast** — распространение размеров матрицы
- **Балансировка нагрузки** — расчет распределения элементов
- **MPI_Scatterv** — распределение блоков матрицы
- **Локальные вычисления** — суммирование элементов блока
- **MPI_Allreduce** — глобальное суммирование с операцией MPI_SUM

#### PostProcessingImpl()
- Каждый процесс имеет готовый результат
- Дополнительные коммуникации не требуются

### Ключевые особенности реализации

- **Децентрализованная архитектура** — равноправные процессы
- **Коллективные операции** — эффективная коммуникация
- **Автоматическая балансировка** — минимизация дисбаланса нагрузки
- **Согласованность данных** — идентичные результаты на всех процессах
- **Масштабируемость** — поддержка произвольного числа процессов

---

## Тестирование

### Результаты производительности (матрица 10000×10000)

#### Время выполнения `task_run` (секунды)

| Режим | Процессы | Время | Ускорение | Эффективность |
|-------|----------|-------|-----------|---------------|
| SEQ   | 1        | 0.057 | 1.00      | —             |
| MPI   | 1        | 0.518 | 0.11      | 0.11          |
| MPI   | 2        | 0.402 | 0.14      | 0.07          |
| MPI   | 4        | 0.354 | 0.16      | 0.04          |

#### Время выполнения `pipeline` (секунды)

| Режим | Процессы | Время | Ускорение | Эффективность |
|-------|----------|-------|-----------|---------------|
| SEQ   | 1        | 0.062 | 1.00      | —             |
| MPI   | 1        | 0.487 | 0.13      | 0.13          |
| MPI   | 2        | 0.427 | 0.15      | 0.07          |
| MPI   | 4        | 0.405 | 0.15      | 0.04          |

### Анализ результатов

#### Основные наблюдения:
- SEQ-версия значительно превосходит MPI по производительности
- Отсутствие реального ускорения от параллелизации
- Низкая эффективность использования процессов (4-13%)
- Незначительное улучшение при увеличении числа процессов

#### Причины низкой производительности MPI:
- **Высокие накладные расходы** на распределение данных (MPI_Scatterv)
- **Доминирование коммуникационных затрат** над вычислительными
- **Низкая коммуникационная эффективность** для операции суммирования

### Проверка корректности
Все тесты успешно пройдены, что подтверждает:
- Корректность реализации SEQ и MPI алгоритмов
- Полное совпадение результатов вычислений
- Отсутствие логических ошибок в реализации

---

## Заключение

В ходе лабораторной работы были успешно реализованы последовательный и параллельный алгоритмы вычисления суммы элементов матрицы. Однако параллельная версия на основе MPI не показала ожидаемого ускорения по сравнению с последовательной реализацией.

Анализ результатов выявил, что для задачи суммирования элементов матрицы коммуникационные издержки MPI существенно превосходят вычислительную нагрузку. Наибольшие затраты времени связаны с операциями распределения данных (MPI_Scatterv) и глобальной редукции (MPI_Allreduce).

Полученные результаты указывают на необходимость применения параллельных подходов для более сложных вычислительных задач, где время вычислений значительно превышает коммуникационные затраты, либо для обработки матриц существенно больших размеров.

---

## Приложение


# **common.hpp**

```cpp
#pragma once

#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace zhurin_i_matrix_sums {

constexpr double kEpsilon = 1e-10;
using InType = std::tuple<uint32_t, uint32_t, std::vector<double>>;
using OutType = double;
using TestType = std::string;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace zhurin_i_matrix_sums
```

---

# **ops_mpi.hpp**

```cpp
#pragma once

#include "task/include/task.hpp"
#include "zhurin_i_matrix_sums/common/include/common.hpp"

namespace zhurin_i_matrix_sums {

class ZhurinIMatrixSumsMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit ZhurinIMatrixSumsMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace zhurin_i_matrix_sums
```

---

# **ops_mpi.cpp**

```cpp
#include "zhurin_i_matrix_sums/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cstdint>
#include <vector>

#include "zhurin_i_matrix_sums/common/include/common.hpp"

namespace zhurin_i_matrix_sums {

ZhurinIMatrixSumsMPI::ZhurinIMatrixSumsMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0.0;
}

bool ZhurinIMatrixSumsMPI::ValidationImpl() {
  return (static_cast<uint64_t>(std::get<0>(GetInput())) * std::get<1>(GetInput()) == std::get<2>(GetInput()).size()) &&
         (GetOutput() == 0.0);
}

bool ZhurinIMatrixSumsMPI::PreProcessingImpl() {
  GetOutput() = 0.0;
  return true;
}

bool ZhurinIMatrixSumsMPI::RunImpl() {
  int rank = 0;
  int wsize = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &wsize);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int rows = static_cast<int>(std::get<0>(GetInput()));
  int columns = static_cast<int>(std::get<1>(GetInput()));
  
  MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&columns, 1, MPI_INT, 0, MPI_COMM_WORLD);
  int size = rows * columns;

  int elems_per_proc = size / wsize;
  int remainder = size % wsize;
  
  std::vector<int> counts(wsize, 0);
  std::vector<int> displs(wsize, 0);
  
  int displ = 0;
  for (int proc = 0; proc < wsize; proc++) {
    counts[proc] = elems_per_proc + (proc < remainder ? 1 : 0);
    displs[proc] = displ;
    displ += counts[proc];
  }

  std::vector<double> matrix;
  if (rank == 0) {
    matrix = std::get<2>(GetInput());  
  }

  std::vector<double> local_buff(counts[rank], 0);

  MPI_Scatterv(rank == 0 ? matrix.data() : nullptr,  
               counts.data(), displs.data(), MPI_DOUBLE, 
               local_buff.data(), counts[rank], MPI_DOUBLE, 0,
               MPI_COMM_WORLD);

  double local_sum = 0.0;
  for (int i = 0; i < counts[rank]; i++) {
    local_sum += local_buff[i];
  }

  double global_sum = 0.0;
  MPI_Allreduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

  GetOutput() = global_sum;
  return true;
}

bool ZhurinIMatrixSumsMPI::PostProcessingImpl() {
  return true;
}

}  // namespace zhurin_i_matrix_sums
```

---

# **ops_seq.hpp**

```cpp
#pragma once

#include "task/include/task.hpp"
#include "zhurin_i_matrix_sums/common/include/common.hpp"

namespace zhurin_i_matrix_sums {

class ZhurinIMatrixSumsSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit ZhurinIMatrixSumsSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace zhurin_i_matrix_sums
```

---

# **ops_seq.cpp**

```cpp
#include "zhurin_i_matrix_sums/seq/include/ops_seq.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

#include "zhurin_i_matrix_sums/common/include/common.hpp"

namespace zhurin_i_matrix_sums {

ZhurinIMatrixSumsSEQ::ZhurinIMatrixSumsSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0.0;
}

bool ZhurinIMatrixSumsSEQ::ValidationImpl() {
  return (static_cast<uint64_t>(std::get<0>(GetInput())) * std::get<1>(GetInput()) == std::get<2>(GetInput()).size()) &&
         (GetOutput() == 0.0);
}

bool ZhurinIMatrixSumsSEQ::PreProcessingImpl() {
  GetOutput() = 0.0;
  return true;
}

bool ZhurinIMatrixSumsSEQ::RunImpl() {
  auto rows = std::get<0>(GetInput());
  auto columns = std::get<1>(GetInput());
  const auto &matrix = std::get<2>(GetInput());

  double sum = 0.0;
  for (size_t i = 0; i < static_cast<size_t>(rows) * columns; i++) {
    sum += matrix[i];
  }

  GetOutput() = sum;
  return true;
}

bool ZhurinIMatrixSumsSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace zhurin_i_matrix_sums
```


