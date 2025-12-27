# Ленточная горизонтальная схема - разбиение только матрицы А - умножение матрицы на матрицу

- Студент: Перепелкин Ярослав Михайлович, группа 3823Б1ПР1
- Технологии: SEQ | MPI
- Вариант: 13

## 1. Введение
Умножение матриц является базовой операцией линейной алгебры и используется во многих задачах вычислительной математики, машинного обучения и компьютерной графики. Для плотных матриц вычисление каждого элемента результата представляет собой независимое скалярное произведение, что делает задачу хорошо подходящей для распараллеливания.

Цель работы – разработать последовательную и параллельную MPI-реализации умножения матриц с использованием ленточной горизонтальной схемы разбиения, при которой распределяется только матрица A, а матрица B полностью доступна всем процессам.

## 2. Постановка задачи
**Определение задачи:**\
Для двух матриц `A` и `B` требуется вычислить произведение `C = A × B`.

**Ограничения:**
- Входные данные – две непустые плотные матрицы `A` и `B`, хранящие вещественные числа.
- Ширины строк в каждой матрице должны быть одинаковыми.
- Для корректного умножения требуется, чтобы количество столбцов матрицы `A` совпадало с количеством строк матрицы `B`.
- Параллельная реализация использует MPI и должна поддерживать различное количество процессов.
- Результаты последовательной и параллельной версий должны быть идентичны.

## 3. Базовый алгоритм (последовательная версия)
**Входные данные:** две матрицы `A (N × M)` и `B (M × K)`, где размеры матрицы записаны в виде `(<число строк> × <число столбцов>)`.

**Выходные данные:** матрица `C (N × K)`, где размеры матрицы записаны в виде `(<число строк> × <число столбцов>)`.

**Алгоритм последовательной реализации:**
1. Сохранить исходные размеры матриц.
2. Преобразовать матрицу `A` в одномерный массив построчно.
3. Построить транспонированную матрицу `B` и преобразовать её в одномерный массив построчно.
4. Вычислить скалярное произведение каждой строки `i` (от `0` до `N-1`) матрицы `A` с каждым столбцом `j` (от `0` до `K-1`) матрицы `B` и записать в элемент `C[i][j]` результирующей матрицы `C`.

**Сложность алгоритма:** `O(N×M×K)`, где `(N × M)` и `(M × K)` – размеры матриц `A` и `B` соответственно.

Реализация последовательного алгоритма представлена в Приложении 1.

## 4. Схема распараллеливания
Параллельная версия реализует ленточную горизонтальную схему: матрица `A` делится на блоки строк и распределяется между процессами, матрица `B` передаётся целиком на все процессы.

### 4.1. Структура параллельного выполнения
- **Инициализация:** Все процессы работают в `MPI_COMM_WORLD`. Процесс 0 владеет входными данными.
- **Рассылка общих данных:** Процесс 0 рассылает исходные размеры матриц и транспонированную матрицу `B` с помощью операции `MPI_Bcast`.
- **Распределение матрицы A:** Процесс 0 распределяет строки матрицы `A` между всеми процессами с помощью операции `MPI_Scatterv` с балансировкой нагрузки – первые `remainder` процессов получают одну дополнительную строку при неравномерном делении.
- **Локальные вычисления:** Каждый процесс вычисляет свой блок строк результирующей матрицы `C` с использованием скалярных произведений.
- **Агрегация результата:** Результирующая матрица `C` собирается на всех процессах в виде одномерного массива с помощью операции `MPI_Allgatherv`.
- **Формирование итога:** Результирующая матрица `C` восстанавливается из одномерного массива.

### 4.2. Организация процессов
- **Процесс 0** выполняет подготовку данных (преобразование матрицы `A` в одномерный массив, построение транспонированной матрицы `B` и её преобразование в одномерный массив) и инициирует коллективные операции передачи.
- **Все процессы** выполняют вычисления соответствующих строк результирующей матрицы `C` и участвуют в коллективной операции `MPI_Allgatherv`.
- **Результат** агрегации одновременно становится доступен на всех процессах.
- **Сохраняется равноправие процессов** на этапе вычисления после распределения данных.

### 4.3. Псевдокод параллельной реализации
```cpp
bool RunImpl() {
    // [1] Широковещательная рассылка исходных размеров матриц
    MPI_Bcast(N, M, K);

    // [2] Широковещательная рассылка транспонированной матрицы B
    MPI_Bcast(flat_b_t);

    // [3] Распределение строк матрицы A
    local_A = MPI_Scatterv(flat_A_by_rows);

    // [4] Локальное вычисление блоков строк результирующей матрицы C
    local_C = local_A * flat_b_t;

    // [5] Агрегация результата на всех процессах
    flat_C = MPI_Allgatherv(local_C);
    return true;
}
```

```cpp
bool PostProcessingImpl() {
    // Восстановление матрицы C
    C = reshape(flat_C, N, K);
    return true;
}
```

Реализация параллельного алгоритма представлена в Приложении 2.

## 5. Детали реализации

### 5.1. Структура кода
**Ключевые файлы проекта:**
```text
perepelkin_i_matrix_mult_horizontal_strip_only_a/
├── common/
│   └── include/common.hpp     - определения типов данных
├── mpi/
│   ├── include/ops_mpi.hpp    - заголовочный файл MPI-реализации
│   └── src/ops_mpi.cpp        - исходный код параллельной версии
├── seq/
│   ├── include/ops_seq.hpp    - заголовочный файл последовательной версии
│   └── src/ops_seq.cpp        - исходный код последовательной версии
└── tests/
    ├── functional/main.cpp    - функциональные тесты
    └── performance/main.cpp   - тесты производительности
```

**Основные классы реализации:**
- `PerepelkinIMatrixMultHorizontalStripOnlyASEQ` – класс последовательной реализации алгоритма.
- `PerepelkinIMatrixMultHorizontalStripOnlyAMPI` – класс параллельной реализации алгоритма.

**Тестовые классы:**
- `PerepelkinIMatrixMultHorizontalStripOnlyAFuncTestProcesses` – класс функционального тестирования.
- `PerepelkinIMatrixMultHorizontalStripOnlyAPerfTestProcesses` – класс тестирования производительности.

**Интерфейс методов реализации:**
- `ValidationImpl()` – проверка корректности начального состояния и входных данных.
- `PreProcessingImpl()` – подготовительные операции с входными данными.
- `RunImpl()` – основной метод, содержащий реализацию алгоритма.
- `PostProcessingImpl()` – завершающая обработка результатов вычислений.

### 5.2. Особенности реализации и обработка граничных случаев
- **Тип данных:** Элементы матриц имеют тип `double`.
- **Пустые матрицы:** Входные матрицы не должны быть пустыми.
- **Оптимизация доступа к памяти:** Матрица `B` хранится в транспонированном плоском виде, чтобы скалярное произведение строки матрицы `A` и столбца матрицы `B` использовало непрерывные диапазоны памяти и работало эффективно с операцией `std::transform_reduce`.

### 5.3. Использование памяти и коммуникации
**Последовательная версия:**
- **Хранение данных:** `O(N×M + M×K + N×K)` – хранение матриц `A` (`N × M`), `B` (`M × K`), `C` (`N × K`).

**Параллельная версия:**
- **Хранение данных:** `O(2×N×M + P × (M×K + N×K))`
  - Процесс 0 хранит исходные матрицы `A` (`N × M`), `B` (`M × K`) и результирующую матрицу `C` (`N × K`).
  - Каждый из `P` процессов хранит назначенные ему блоки строк матрицы `A` размера `N × M / P` и полные копии матриц `B` (`M × K`), `C` (`N × K`).
- **Обмен данными:**
  - `MPI_Bcast`: `O(M×K)` – передача размеров матриц и транспонированной матрицы `B`.
  - `MPI_Scatterv`: `O(N×M)` – распределение строк матрицы `A` по процессам.
  - `MPI_Allgatherv`: `O(N×K×P)` – сбор всех блоков строк результирующей матрицы `C` и её рассылка всем процессам.

## 6. Тестовая инфраструктура

### 6.1. Аппаратное обеспечение:
| Параметр | Значение                                            |
| -------- | --------------------------------------------------- |
| CPU      | Intel Core i5-12400 (6 cores, 12 threads, 2.50 GHz) |
| RAM      | 32 GB DDR4 (3200 MHz)                               |
| OS       | Ubuntu 24.04.1 LTS on Windows 10 x86_64             |

### 6.2. Программное обеспечение:
| Параметр   | Значение       |
| ---------- | -------------- |
| Компилятор | g++ 13.3.0     |
| MPI        | Open MPI 4.1.6 |
| Сборка     | Release        |

### 6.3. Тестовые данные
**Функциональные тесты:** используют заранее заданные матрицы и ожидаемый результат.

**Тесты производительности:** данные генерируются программно по следующему алгоритму:
1. Сгенерировать матрицу `A` размера `N × M` со случайными целыми значениями в диапазоне `[-1000; 1000]`.
2. Сгенерировать матрицу `B` размера `M × K` аналогичным образом.
3. Вычислить ожидаемый результат (матрицу `C`) последовательным алгоритмом.

Реализация генерации тестовых данных представлена в Приложении 3.

## 7. Результаты и обсуждение

### 7.1 Корректность
Для проверки корректности работы алгоритма проводилось функциональное тестирование на основе Google Test Framework, которое включало:
- Проверку соответствия результатов заранее определённым ожидаемым значениям.
- Анализ работы алгоритма на граничных случаях:
  - Нулевые матрицы.
  - Единичные матрицы.
  - Вектор-строки и вектор-столбцы.
  - Несбалансированность числа строк матрицы относительно числа процессов.

**Перечень функциональных тестов:**
| Название теста | Размер A | Размер B | Описание теста |
| --- | --- | --- | --- |
| `basic_sample` | 2x2 | 2x2 | Базовый пример |
| `nontrivial_mixed` | 4x4 | 4x4 | Нетривиальный пример со смешанными значениями |
| `zeros_both` | 2x2 | 2x2 | Обе матрицы нулевые |
| `zeros_only_a` | 3x2 | 2x2 | Нулевая `A`, ненулевая `B` |
| `zeros_only_b` | 3x4 | 4x2 | Нулевая `B`, ненулевая `A` |
| `identity_both` | 2x2 | 2x2 | Обе матрицы единичные |
| `identity_only_a` | 3x3 | 3x3 | Единичная `A`, произвольная `B` |
| `identity_only_b` | 3x3 | 3x3 | Произвольная `A`, единичная `B` |
| `row_times_matrix` | 1x4 | 4x3 | Умножение вектор-строки на матрицу |
| `col_times_row` | 4x1 | 1x3 | Умножение вектор-столбца на вектор-строку |
| `row_times_col` | 1x3 | 3x1 | Умножение вектор-строки на вектор-столбец |
| `single_elem` | 1x1 | 1x1 | Матрицы размера 1 |
| `uneven_rows` | 5x3 | 3x2 | Число строк матрицы `A` не кратно числу процессов |
| `negative_values` | 2x2 | 2x2 | Только отрицательные элементы |
| `alternating_signs` | 2x3 | 3x2 | Чередующиеся знаки |
| `mixed_signs` | 3x2 | 2x3 | Смешанные знаки |

Результаты функционального тестирования подтвердили корректность реализации алгоритма – все тестовые сценарии были успешно пройдены.

### 7.2 Производительность

**Параметры тестирования:**
- **Данные:** матрица `A` размера `1000 x 2000`, матрица `B` размера `2000 x 2000`.
- **Метрики:**
  - Абсолютное время выполнения.
  - Ускорение относительно последовательной версии.
  - Эффективность параллелизации – рассчитывается как `(ускорение / количество процессов) * 100%`.
- **Сценарии измерения:**
  - **Полный цикл (pipeline)** – измерение времени выполнения всей программы (`Validation`, `PreProcessing`, `RunImpl`, `PostProcessing`).
  - **Только вычислительная часть (task_run)** – измерение времени только этапа выполнения алгоритма (`RunImpl`).

**Результаты полного цикла выполнения:**
| Режим | Процессы | Время, с | Ускорение | Эффективность, % |
| ----- | -------- | -------- | --------- | ---------------- |
| seq   | 1        | 1.685517 | 1.000     | N/A              |
| mpi   | 2        | 1.018857 | 1.654     | 82.72            |
| mpi   | 4        | 0.783803 | 2.150     | 53.76            |

**Результаты вычислительной части:**
| Режим | Процессы | Время, с | Ускорение | Эффективность, % |
| ----- | -------- | -------- | --------- | ---------------- |
| seq   | 1        | 1.680378 | 1.000     | N/A              |
| mpi   | 2        | 1.009925 | 1.664     | 83.20            |
| mpi   | 4        | 0.775680 | 2.166     | 54.16            |

**Анализ результатов (на основе сценария task_run):**
- **Производительность:** MPI-реализация демонстрирует ускорение относительно последовательной версии (ускорение 1.66x на 2 процессах и 2.17x на 4 процессах).
- **Эффективность параллелизации:** На 2 процессах эффективность составляет 83% с последующим снижением до 54% при использовании 4 процессов.
- **Ограничения масштабируемости:** При увеличении числа процессов возрастают затраты на коммуникацию, что приводит к снижению эффективности.

## 8. Выводы
**Реализация и тестирование:**
- Успешно разработаны последовательная и параллельная MPI-версии умножения матриц.
- Проведенное функциональное тестирование подтвердило корректность работы обеих версий на различных наборах данных.

**Результаты производительности:**
- Параллельная реализация алгоритма показывает увеличение производительности относительно последовательной версии: ускорение составляет 1.66x при использовании 2 процессов и 2.17x при 4 процессах.
- Эффективность параллелизации составляет 83% на 2 процессах с последующим снижением до 54% на 4 процессах.

**Выявленные проблемы:**
- При увеличении числа процессов эффективность снижается из-за накладных расходов на обмен данными (`MPI_Bcast` матрицы `B` и `MPI_Allgatherv` результата).

## 9. Источники
1. Документация по курсу «Параллельное программирование» // Parallel Programming Course URL: https://learning-process.github.io/parallel_programming_course/ru/index.html (дата обращения: 08.12.2025).
2. Сысоев А. В. «Коллективные и парные взаимодействия» // Лекции по дисциплине «Параллельное программирование для кластерных систем». — 2025.
3. Коллективные функции MPI // Microsoft URL: https://learn.microsoft.com/ru-ru/message-passing-interface/mpi-collective-functions (дата обращения: 08.12.2025).
4. std::transform_reduce // cppreference.com URL: https://en.cppreference.com/w/cpp/algorithm/transform_reduce.html (дата обращения: 09.12.2025).

## Приложения

### Приложение №1. Реализация последовательной версии алгоритма
```cpp
bool PerepelkinIMatrixMultHorizontalStripOnlyASEQ::PreProcessingImpl() {
  const auto &[matrix_a, matrix_b] = GetInput();

  // Store original sizes
  height_a_ = matrix_a.size();
  height_b_ = matrix_b.size();
  width_a_ = matrix_a[0].size();
  width_b_ = matrix_b[0].size();

  // Flatten matrix A
  flat_a_.reserve(width_a_ * height_a_);
  for (const auto &row : matrix_a) {
    flat_a_.insert(flat_a_.end(), row.begin(), row.end());
  }

  // Create transposed-and-flattened matrix B
  flat_b_t_.resize(width_b_ * height_b_);
  for (size_t row = 0; row < height_b_; row++) {
    for (size_t col = 0; col < width_b_; col++) {
      flat_b_t_[(col * height_b_) + row] = matrix_b[row][col];
    }
  }

  return true;
}
```

```cpp
bool PerepelkinIMatrixMultHorizontalStripOnlyASEQ::RunImpl() {
  auto &output = GetOutput();
  output.resize(height_a_);
  for (auto &row : output) {
    row.resize(width_b_);
  }

  for (size_t i = 0; i < height_a_; ++i) {
    const auto a_it = flat_a_.begin() + static_cast<DiffT>(i * width_a_);
    const auto a_end = a_it + static_cast<DiffT>(width_a_);
    for (size_t j = 0; j < width_b_; ++j) {
      const auto b_it = flat_b_t_.begin() + static_cast<DiffT>(j * width_a_);
      output[i][j] = std::transform_reduce(a_it, a_end, b_it, 0.0, std::plus<>(), std::multiplies<>());
    }
  }

  return true;
}
```

### Приложение №2. Реализация параллельной версии алгоритма
```cpp
bool PerepelkinIMatrixMultHorizontalStripOnlyAMPI::PreProcessingImpl() {
  if (proc_rank_ == 0) {
    const auto &[matrix_a, matrix_b] = GetInput();

    // Store original sizes on root
    height_a_ = matrix_a.size();
    height_b_ = matrix_b.size();
    width_a_ = matrix_a[0].size();
    width_b_ = matrix_b[0].size();

    // Flatten matrix A
    flat_a_.reserve(width_a_ * height_a_);
    for (const auto &row : matrix_a) {
      flat_a_.insert(flat_a_.end(), row.begin(), row.end());
    }

    // Create transposed-and-flattened matrix B
    flat_b_t_.resize(width_b_ * width_a_);
    for (size_t row = 0; row < width_a_; row++) {
      for (size_t col = 0; col < width_b_; col++) {
        flat_b_t_[(col * width_a_) + row] = matrix_b[row][col];
      }
    }
  }

  return true;
}
```

```cpp
bool PerepelkinIMatrixMultHorizontalStripOnlyAMPI::RunImpl() {
  // [1] Broadcast matrix sizes and matrix B
  BcastMatrixSizes();
  BcastMatrixB();

  // [2] Distribute matrix A
  std::vector<double> local_a;
  std::vector<int> rows_per_rank;
  const size_t local_rows = DistributeMatrixA(local_a, rows_per_rank);

  // [3] Local computation of matrix C
  std::vector<double> local_c(local_rows * width_b_);
  for (size_t row_a = 0; row_a < local_rows; row_a++) {
    const auto a_it = local_a.begin() + static_cast<DiffT>(row_a * width_a_);
    const auto a_end = a_it + static_cast<DiffT>(width_a_);

    for (size_t col_b = 0; col_b < width_b_; col_b++) {
      const auto b_it = flat_b_t_.begin() + static_cast<DiffT>(col_b * width_a_);
      local_c[(row_a * width_b_) + col_b] =
          std::transform_reduce(a_it, a_end, b_it, 0.0, std::plus<>(), std::multiplies<>());
    }
  }

  // [4] Gather local results
  GatherAndBcastResult(rows_per_rank, local_c);
  return true;
}
```

```cpp
void PerepelkinIMatrixMultHorizontalStripOnlyAMPI::BcastMatrixSizes() {
  MPI_Bcast(&height_a_, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&height_b_, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&width_a_, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&width_b_, 1, MPI_INT, 0, MPI_COMM_WORLD);
}
```

```cpp
void PerepelkinIMatrixMultHorizontalStripOnlyAMPI::BcastMatrixB() {
  const size_t total_t = width_b_ * height_b_;

  if (proc_rank_ != 0) {
    flat_b_t_.resize(total_t);
  }

  MPI_Bcast(flat_b_t_.data(), static_cast<int>(total_t), MPI_DOUBLE, 0, MPI_COMM_WORLD);
}
```

```cpp
int PerepelkinIMatrixMultHorizontalStripOnlyAMPI::DistributeMatrixA(std::vector<double> &local_a,
                                                                    std::vector<int> &rows_per_rank) {
  // Determine rows per rank
  rows_per_rank.resize(proc_num_);
  const int base_rows = static_cast<int>(height_a_ / proc_num_);
  const int remainder_rows = static_cast<int>(height_a_ % proc_num_);
  for (int i = 0; i < proc_num_; i++) {
    rows_per_rank[i] = base_rows + (i < remainder_rows ? 1 : 0);
  }

  // Prepare counts and displacements
  std::vector<int> counts(proc_num_);
  std::vector<int> displacements(proc_num_);
  for (int i = 0, offset = 0; i < proc_num_; i++) {
    counts[i] = rows_per_rank[i] * static_cast<int>(width_a_);
    displacements[i] = offset;
    offset += counts[i];
  }

  const int local_a_size = rows_per_rank[proc_rank_] * static_cast<int>(width_a_);
  local_a.resize(local_a_size);

  MPI_Scatterv(flat_a_.data(), counts.data(), displacements.data(), MPI_DOUBLE, local_a.data(), local_a_size,
               MPI_DOUBLE, 0, MPI_COMM_WORLD);

  return rows_per_rank[proc_rank_];
}
```

```cpp
void PerepelkinIMatrixMultHorizontalStripOnlyAMPI::GatherAndBcastResult(const std::vector<int> &rows_per_rank,
                                                                        const std::vector<double> &local_c) {
  std::vector<int> counts(proc_num_);
  std::vector<int> displacements(proc_num_);
  for (int i = 0, offset = 0; i < proc_num_; i++) {
    counts[i] = rows_per_rank[i] * static_cast<int>(width_b_);
    displacements[i] = offset;
    offset += counts[i];
  }

  flat_c_.resize(height_a_ * width_b_);

  MPI_Allgatherv(local_c.data(), counts[proc_rank_], MPI_DOUBLE, flat_c_.data(), counts.data(), displacements.data(),
                 MPI_DOUBLE, MPI_COMM_WORLD);
}
```

```cpp
bool PerepelkinIMatrixMultHorizontalStripOnlyAMPI::PostProcessingImpl() {
  auto &output = GetOutput();
  output.resize(height_a_);
  for (auto &row : output) {
    row.resize(width_b_);
  }

  for (size_t i = 0; i < height_a_; i++) {
    for (size_t j = 0; j < width_b_; j++) {
      output[i][j] = flat_c_[(i * width_b_) + j];
    }
  }

  return true;
}
```

### Приложение №3. Генерация тестовых данных для тестов производительности
```cpp
static std::tuple<std::vector<std::vector<double>>, std::vector<std::vector<double>>,
                  std::vector<std::vector<double>>>
GenerateTestData(size_t rows_a, size_t cols_a, size_t cols_b, unsigned int seed) {
  std::mt19937 gen(seed);
  std::uniform_int_distribution<int> val_dist(-1000, 1000);

  std::vector<std::vector<double>> matrix_a(rows_a, std::vector<double>(cols_a));
  std::vector<std::vector<double>> matrix_b(cols_a, std::vector<double>(cols_b));

  for (size_t i = 0; i < rows_a; i++) {
    for (size_t j = 0; j < cols_a; j++) {
      matrix_a[i][j] = static_cast<double>(val_dist(gen));
    }
  }

  for (size_t i = 0; i < cols_a; i++) {
    for (size_t j = 0; j < cols_b; j++) {
      matrix_b[i][j] = static_cast<double>(val_dist(gen));
    }
  }

  std::vector<std::vector<double>> matrix_c(rows_a, std::vector<double>(cols_b, 0.0));

  for (size_t i = 0; i < rows_a; i++) {
    for (size_t j = 0; j < cols_b; j++) {
      double tmp = 0.0;
      for (size_t k = 0; k < cols_a; k++) {
        tmp += matrix_a[i][k] * matrix_b[k][j];
      }
      matrix_c[i][j] = tmp;
    }
  }

  return {matrix_a, matrix_b, matrix_c};
}
```