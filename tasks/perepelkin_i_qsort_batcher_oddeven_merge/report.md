# Быстрая сортировка с четно-нечетным слиянием Бэтчера.

- Студент: Перепелкин Ярослав Михайлович, группа 3823Б1ПР1
- Технологии: SEQ | MPI
- Вариант: 15

## 1. Введение
Сортировка массивов является одной из ключевых задач алгоритмической обработки данных и часто используется как базовый этап в более сложных вычислительных алгоритмах. Для последовательных вычислений широко применяется быстрая сортировка, обеспечивающая высокую среднюю производительность.

При переходе к параллельным вычислениям производительность может быть увеличена за счёт одновременной сортировки различных участков массива с последующим объединением отсортированных подмассивов. Для эффективного выполнения этапа слияния в параллельной среде может использоваться четно-нечетное слияние Бэтчера, позволяющее организовать независимые операции сравнения и обмена данных.

Цель работы – разработать параллельную MPI-реализацию быстрой сортировки с использованием четно-нечетного слияния Бэтчера и провести сравнительный анализ её производительности с последовательной версией при различном количестве процессов.

## 2. Постановка задачи
**Определение задачи:**\
Требуется отсортировать массив вещественных чисел в неубывающем порядке.

**Ограничения:**
- Входные данные – непустой массив вещественных чисел (`double`).
- Параллельная реализация использует MPI и должна поддерживать различное количество процессов.
- Результаты последовательной и параллельной версий должны быть идентичны.

## 3. Базовый алгоритм (последовательная версия)
**Входные данные:** массив вещественных чисел размера `N`.

**Выходные данные:** отсортированный массив размера `N`.

**Описание последовательной реализации:**\
В качестве последовательного алгоритма сортировки используется стандартная библиотечная функция `std::qsort`, реализующая эффективный алгоритм сортировки общего назначения. Данная реализация применяется в работе в качестве базового варианта для сравнения с параллельной версией алгоритма.

**Сложность алгоритма:** `O(N log N)` в среднем, где `N` – размер массива.

Реализация последовательного алгоритма представлена в Приложении 1.

## 4. Схема распараллеливания
Параллельная версия алгоритма сортировки основана на распределении входного массива между процессами, независимой локальной сортировке подмассивов и последующем глобальном слиянии отсортированных блоков с использованием четно-нечетного слияния Бэтчера. Обмен данными между процессами и синхронизация выполняются с помощью коллективных и точечных операций MPI.

### 4.1. Структура параллельного выполнения
- **Инициализация:** Все процессы запускаются в одном коммуникаторе `MPI_COMM_WORLD`. Процесс 0 владеет входным массивом.
- **Подготовка данных:** Процесс 0 определяет исходный размер массива и дополняет его до размера, кратного числу процессов, фиктивными элементами, не влияющими на результат сортировки.
- **Распределение данных:** Дополненный массив равномерно распределяется между процессами с помощью операции `MPI_Scatterv`.
- **Локальная сортировка:** Каждый процесс независимо сортирует свой локальный подмассив стандартной функцией сортировки.
- **Глобальное слияние:** Отсортированные подмассивы объединяются с использованием сети компараторов, построенной на основе четно-нечетного слияния Бэтчера, с обменом данных между соответствующими парами процессов.
- **Сбор результата:** Отсортированные блоки собираются на процессе 0 с помощью операции `MPI_Gatherv`, после чего отбрасываются добавленные фиктивные элементы.
- **Рассылка результата:** Итоговый отсортированный массив передается всем процессам с помощью операции `MPI_Bcast`.

### 4.2. Организация процессов
- **Процесс 0** выполняет подготовку входных данных (дополнение массива) и инициирует коллективные операции передачи.
- **Все процессы** участвуют в локальной сортировке, обработке компараторов и обмене данными на этапе распределения данных и глобального слияния.
- **Равноправие процессов** сохраняется на этапах локальных вычислений и слияния, где каждый процесс выполняет одинаковый набор операций в соответствии со своей ролью в сети компараторов.

### 4.3. Четно-нечетное слияние Бэтчера
В параллельной реализации алгоритма каждый процесс владеет своим локальным подмассивом. Для глобального объединения отсортированных подмассивов строится сеть компараторов, где компаратор — это пара процессов, между которыми необходимо выполнить слияние их блоков данных.

**Формирование компараторов:**  
Процесс начинается с создания списка всех процессов. Далее вызывается рекурсивная функция, которая формирует компараторы для всего множества процессов.

- **Этап разделения:** множество процессов делится на две части. Для каждой части рекурсивно вызывается тот же алгоритм построения компараторов, т.е. формируются компараторы внутри подгруппы процессов. После завершения рекурсивной обработки обе подгруппы готовы к глобальному слиянию между собой.

- **Этап слияния:** вызывается специальная функция для двух подгрупп процессов, в которой:
  1. Каждый подмассив процессов делится на две части с чётными и нечётными индексами.
  2. Для каждой части рекурсивно продолжается разбиение процессов на подгруппы с чётными и нечётными позициями, пока не останутся пары процессов для непосредственного слияния.
  3. Когда остаётся пара процессов для слияния, формируется компаратор — фиксируется пара процессов, которые должны обменяться данными для слияния их подмассивов.
  4. На более высоких уровнях выполняется соединение соседних процессов, формируя все необходимые компараторы для корректного глобального слияния.

**Использование компараторов:**  
После формирования списка компараторов каждый процесс проходит по списку. Если процесс участвует в текущем компараторе, он обменивается данными с соответствующим процессом и выполняет слияние подмассивов. В результате у процесса остаётся либо меньшая, либо большая часть объединённого блока, в зависимости от позиции в компараторе. После обработки всех компараторов глобальный порядок элементов в массиве восстанавливается.

Таким образом, сеть компараторов определяет точный порядок обменов и слияний между процессами, обеспечивая корректную параллельную сортировку всего массива.

### 4.4. Псевдокод параллельной реализации
```cpp
bool RunImpl() {
    // [1.1] Определение исходного и дополненного размеров массива
    if (proc_rank == 0) {
        original_size = input.size();
        padded_size = PadToProcessCount(original_size);
    }

    // [1.2] Широковещательная рассылка размеров массива
    MPI_Bcast(original_size, padded_size);

    // [2] Распределение дополненного массива между процессами
    local_data = ScatterData(padded_input);

    // [3] Локальная сортировка
    sort(local_data);

    // [4.1] Построение сети компараторов
    comparators = BuildOddEvenMergeComparators(proc_num);

    // [4.2] Последовательная обработка компараторов
    for each (p_i, p_j) in comparators {
        if (proc_rank == p_i or proc_rank == p_j) {
            peer_data = ExchangeDataWithPeer();
            local_data = MergeAndKeepPart(local_data, peer_data);
        }
    }

    // [5] Сбор отсортированных блоков на процессе 0 и удаление фиктивных элементов
    result = MPI_Gatherv(local_data);
    result = RemoveDummyElems(result, original_size);
    
    // [6] Рассылка результата всем процессам
    MPI_Bcast(result);
    return true;
}
```

Реализация параллельного алгоритма представлена в Приложении 2.

## 5. Детали реализации

### 5.1. Структура кода
**Ключевые файлы проекта:**
```text
perepelkin_i_qsort_batcher_oddeven_merge/
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
- `PerepelkinIQsortBatcherOddEvenMergeSEQ` – класс последовательной реализации алгоритма.
- `PerepelkinIQsortBatcherOddEvenMergeMPI` – класс параллельной реализации алгоритма.

**Тестовые классы:**
- `PerepelkinIQsortBatcherOddEvenMergeFuncTests` – класс функционального тестирования.
- `PerepelkinIQsortBatcherOddEvenMergePerfTests` – класс тестирования производительности.

**Интерфейс методов реализации:**
- `ValidationImpl()` – проверка корректности начального состояния и входных данных.
- `PreProcessingImpl()` – подготовительные операции с входными данными.
- `RunImpl()` – основной метод, содержащий реализацию алгоритма.
- `PostProcessingImpl()` – завершающая обработка результатов вычислений.

### 5.2. Особенности реализации и обработка граничных случаев
- **Тип данных:** Элементы массива имеют тип `double`.
- **Пустой массив:** Обработка пустого входа – возвращение пустого выхода.
- **Некратный размер массива:** Дополнение массива фиктивными значениями, не влияющими на сортировку, для кратности количеству процессов.
- **Локальные сортировки:** Используется стандартная библиотечная функция `std::qsort`.
- **Рекурсия:** Явная рекурсия в построении компараторов заменена на стек для избежания переполнения стека рекурсии.

### 5.3. Использование памяти и коммуникации
**Последовательная версия:**
- **Хранение данных:** `O(N)` – хранение исходного массива размера `N`.

**Параллельная версия:**
- **Хранение данных:** `O(N × (2 + P))`
  - Процесс 0 хранит исходный массив и результирующий отсортированный массив размерами `N`.
  - Каждый из `P` процессов хранит назначенный ему подмассив размера `N/P` и результирующий отсортированный массив размера `N`.
- **Обмен данными:**
  - `MPI_Bcast`: `O(P×N)` – передача размеров матрицы и результирующего отсортированного массива.
  - `MPI_Scatterv`: `O(N)` – распределение подмассивов по процессам.
  - `MPI_Sendrecv`: `O(N×log²P)` – передача подмассивов между процессами при слиянии.
  - `MPI_Gatherv`: `O(N)` – сбор отсортированного массива на процессе 0.
  
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
**Функциональные тесты:** используют заранее заданные массивы и ожидаемый результат.

**Тесты производительности:** данные генерируются программно по следующему алгоритму:
1. Сгенерировать базовый массив размера `N` со случайными вещественными значениями в диапазоне `[-1000.0; 1000.0]`.
2. Создать тестовый массив, повторяя базовый массив `M` раз.

Реализация генерации тестовых данных представлена в Приложении 3.

## 7. Результаты и обсуждение

### 7.1 Корректность
Для проверки корректности работы алгоритма проводилось функциональное тестирование на основе Google Test Framework, которое включало:
- Проверку соответствия результатов заранее определённым ожидаемым значениям.
- Анализ работы алгоритма на граничных случаях:
  - Пустые массивы.
  - Массивы из одного элемента.
  - Отсортированные массивы.
  - Массивы, отсортированные в обратном порядке.

**Перечень функциональных тестов:**
| Название теста | Описание теста |
| -------------- | -------------- |
| `empty` | Пустой массив |
| `single` | Массив из одного элемента |
| `duplicates_and_negatives` | Массив с дубликатами и отрицательными числами |
| `already_sorted` | Уже отсортированный массив |
| `reverse_sorted` | Массив, отсортированный в обратном порядке |
| `mixed` | Массив со смешанными значениями и дубликатами |
| `odd_size` | Массив нечётной длины |
| `even_size` | Массив чётной длины |
| `many_duplicates` | Массив с множеством одинаковых элементов |
| `wide_range` | Массив с "большим" диапазоном значений |
| `decimal_precision` | Массив с близкими по значению вещественными числами |
| `longer_random_like` | Более длинный массив, имитирующий случайные данные |

Результаты функционального тестирования подтвердили корректность реализации алгоритма – все тестовые сценарии были успешно пройдены.

### 7.2 Производительность
**Параметры тестирования:**
- **Данные:** массив размера 16 миллионов элементов.
- **Метрики:**
  - Абсолютное время выполнения.
  - Ускорение относительно последовательной версии.
  - Эффективность параллелизации – рассчитывается как `(ускорение / количество процессов) × 100%`.
- **Сценарии измерения:**
  - **Полный цикл (pipeline)** – измерение времени выполнения всей программы (`Validation`, `PreProcessing`, `RunImpl`, `PostProcessing`).
  - **Только вычислительная часть (task_run)** – измерение времени только этапа выполнения алгоритма (`RunImpl`).

**Результаты полного цикла выполнения:**
| Режим | Процессы | Время, с | Ускорение | Эффективность, % |
| ----- | -------- | -------- | --------- | ---------------- |
| seq   | 1        | 0.859260 | 1.000     | N/A              |
| mpi   | 2        | 0.497903 | 1.726     | 86.29            |
| mpi   | 4        | 0.317151 | 2.709     | 67.73            |
| mpi   | 7        | 0.258891 | 3.319     | 47.41            |
| mpi   | 8        | 0.238491 | 3.603     | 45.04            |

**Результаты вычислительной части:**
| Режим | Процессы | Время, с | Ускорение | Эффективность, % |
| ----- | -------- | -------- | --------- | ---------------- |
| seq   | 1        | 0.850949 | 1.000     | N/A              |
| mpi   | 2        | 0.495722 | 1.717     | 85.83            |
| mpi   | 4        | 0.306671 | 2.775     | 69.37            |
| mpi   | 7        | 0.257595 | 3.303     | 47.19            |
| mpi   | 8        | 0.237768 | 3.579     | 44.74            |

**Анализ результатов (на основе сценария task_run):**
- **Производительность:** MPI-реализация демонстрирует ускорение относительно последовательной версии (1.72× на 2 процессах, 3.58× на 8 процессах).
- **Эффективность параллелизации:** На 2 процессах эффективность составляет 85.83% с последующим снижением до 44.74% при использовании 8 процессов.
- **Ограничения масштабируемости:** При увеличении числа процессов возрастают затраты на коммуникацию, что приводит к снижению эффективности.

## 8. Выводы
**Реализация и тестирование:**
- Успешно разработаны последовательная и параллельная MPI-версии сортировки массивов.
- Проведенное функциональное тестирование подтвердило корректность работы обеих версий на различных наборах данных.

**Результаты производительности:**
- Параллельная реализация алгоритма показывает увеличение производительности относительно последовательной версии: ускорение составляет 1.72× при использовании 2 процессов и 3.58× при 8 процессах.
- Эффективность параллелизации составляет 85.83% на 2 процессах с последующим снижением до 44.74% на 8 процессах.

**Выявленные проблемы:**
- При увеличении числа процессов эффективность снижается из-за накладных расходов на обмен данными между процессами и увеличения числа слияний, которые необходимо выполнить в сети компараторов.

## 9. Источники
1. Документация по курсу «Параллельное программирование» // Parallel Programming Course URL: https://learning-process.github.io/parallel_programming_course/ru/index.html (дата обращения: 15.12.2025).
2. Сысоев А. В. «Коллективные и парные взаимодействия» // Лекции по дисциплине «Параллельное программирование для кластерных систем». — 2025.
3. Коллективные функции MPI // Microsoft URL: https://learn.microsoft.com/ru-ru/message-passing-interface/mpi-collective-functions (дата обращения: 15.12.2025).
4. Сеть обменной сортировки со слиянием Бэтчера // Хабр URL: https://habr.com/ru/articles/275889/ (дата обращения: 16.12.2025).
5. Batcher odd–even mergesort // Wikipedia URL: https://en.wikipedia.org/wiki/Batcher_odd%E2%80%93even_mergesort (дата обращения: 16.12.2025).
6. qsort // Microsoft Learn URL: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/qsort?view=msvc-170 (дата обращения: 22.12.2025).

## Приложения

### Приложение №1. Реализация последовательной версии алгоритма
```cpp
bool PerepelkinIQsortBatcherOddEvenMergeSEQ::RunImpl() {
  const auto &data = GetInput();
  if (data.empty()) {
    return true;
  }

  std::vector<double> buffer = data;
  std::qsort(buffer.data(), buffer.size(), sizeof(double), [](const void *a, const void *b) {
    double arg1 = *static_cast<const double *>(a);
    double arg2 = *static_cast<const double *>(b);
    if (arg1 < arg2) {
      return -1;
    }
    if (arg1 > arg2) {
      return 1;
    }
    return 0;
  });

  GetOutput() = std::move(buffer);
  return true;
}
```

### Приложение №2. Реализация параллельной версии алгоритма
```cpp
bool PerepelkinIQsortBatcherOddEvenMergeMPI::RunImpl() {
  // [1] Broadcast original and padded sizes
  size_t original_size = 0;
  size_t padded_size = 0;

  BcastSizes(original_size, padded_size);

  if (original_size == 0) {
    return true;
  }

  // [2] Distribute data
  std::vector<double> padded_input;
  if (proc_rank_ == 0) {
    padded_input = GetInput();
    if (padded_size > original_size) {
      padded_input.resize(padded_size, std::numeric_limits<double>::infinity());
    }
  }

  std::vector<int> counts;
  std::vector<int> displs;
  std::vector<double> local_data;
  DistributeData(padded_size, padded_input, counts, displs, local_data);

  // [3] Local sort
  std::qsort(local_data.data(), local_data.size(), sizeof(double), [](const void *a, const void *b) {
    double arg1 = *static_cast<const double *>(a);
    double arg2 = *static_cast<const double *>(b);
    if (arg1 < arg2) {
      return -1;
    }
    if (arg1 > arg2) {
      return 1;
    }
    return 0;
  });

  // [4] Global merge via comparator network
  std::vector<std::pair<int, int>> comparators;
  BuildComparators(comparators);
  ProcessComparators(counts, local_data, comparators);

  // [5] Gather result on root
  std::vector<double> gathered;
  if (proc_rank_ == 0) {
    gathered.resize(padded_size);
  }

  MPI_Gatherv(local_data.data(), static_cast<int>(local_data.size()), MPI_DOUBLE, gathered.data(), counts.data(),
              displs.data(), MPI_DOUBLE, 0, MPI_COMM_WORLD);

  if (proc_rank_ == 0) {
    gathered.resize(original_size);
    GetOutput() = std::move(gathered);
  }

  // [6] Bcast output to all processes
  GetOutput().resize(original_size);
  MPI_Bcast(GetOutput().data(), static_cast<int>(original_size), MPI_DOUBLE, 0, MPI_COMM_WORLD);

  return true;
}
```

```cpp
void PerepelkinIQsortBatcherOddEvenMergeMPI::BcastSizes(size_t &original_size, size_t &padded_size) {
  if (proc_rank_ == 0) {
    original_size = GetInput().size();
    const size_t remainder = original_size % proc_num_;
    padded_size = original_size + (remainder == 0 ? 0 : (proc_num_ - remainder));
  }

  MPI_Bcast(&original_size, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
  MPI_Bcast(&padded_size, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
}
```

```cpp
void PerepelkinIQsortBatcherOddEvenMergeMPI::DistributeData(const size_t &padded_size,
                                                            const std::vector<double> &padded_input,
                                                            std::vector<int> &counts, std::vector<int> &displs,
                                                            std::vector<double> &local_data) const {
  const int base_size = static_cast<int>(padded_size / proc_num_);

  counts.resize(proc_num_);
  displs.resize(proc_num_);

  for (int i = 0, offset = 0; i < proc_num_; i++) {
    counts[i] = base_size;
    displs[i] = offset;
    offset += base_size;
  }

  const int local_size = counts[proc_rank_];
  local_data.resize(local_size);

  MPI_Scatterv(padded_input.data(), counts.data(), displs.data(), MPI_DOUBLE, local_data.data(), local_size, MPI_DOUBLE,
               0, MPI_COMM_WORLD);
}
```

```cpp
void PerepelkinIQsortBatcherOddEvenMergeMPI::BuildComparators(std::vector<std::pair<int, int>> &comparators) const {
  std::vector<int> procs(proc_num_);
  for (int i = 0; i < proc_num_; i++) {
    procs[i] = i;
  }

  BuildStageB(procs, comparators);
}
```

```cpp
void PerepelkinIQsortBatcherOddEvenMergeMPI::BuildStageB(const std::vector<int> &procs,
                                                         std::vector<std::pair<int, int>> &comparators) {
  // Task: (subarray, is_merge_phase)
  std::stack<std::pair<std::vector<int>, bool>> tasks;
  tasks.emplace(procs, false);

  while (!tasks.empty()) {
    auto [current, is_merge] = tasks.top();
    tasks.pop();

    if (current.size() <= 1) {
      continue;
    }

    // Split phase: divide and schedule children
    auto mid = static_cast<DiffT>(current.size() / 2);
    std::vector<int> left(current.begin(), current.begin() + mid);
    std::vector<int> right(current.begin() + mid, current.end());

    if (is_merge) {
      BuildStageS(left, right, comparators);
      continue;
    }

    // Schedule merge after children complete
    tasks.emplace(current, true);
    tasks.emplace(right, false);  // Right child first (LIFO order)
    tasks.emplace(left, false);
  }
}
```

```cpp
void PerepelkinIQsortBatcherOddEvenMergeMPI::BuildStageS(const std::vector<int> &up, const std::vector<int> &down,
                                                         std::vector<std::pair<int, int>> &comparators) {
  // Task: (part_up, part_down, is_merge_phase)
  std::stack<std::tuple<std::vector<int>, std::vector<int>, bool>> tasks;
  tasks.emplace(up, down, false);

  while (!tasks.empty()) {
    auto [part_up, part_down, is_merge] = tasks.top();
    tasks.pop();
    const size_t total_size = part_up.size() + part_down.size();

    if (total_size <= 1) {
      continue;
    }
    if (total_size == 2) {
      comparators.emplace_back(part_up[0], part_down[0]);
      continue;
    }

    if (!is_merge) {
      // Split phase: separate odd/even indices
      auto [a_odd, a_even] = Split(part_up);
      auto [b_odd, b_even] = Split(part_down);

      // Schedule merge after recursive processing
      tasks.emplace(part_up, part_down, true);
      tasks.emplace(a_even, b_even, false);
      tasks.emplace(a_odd, b_odd, false);
      continue;
    }

    // Merge phase: connect adjacent elements
    std::vector<int> merged;
    merged.reserve(total_size);
    merged.insert(merged.end(), part_up.begin(), part_up.end());
    merged.insert(merged.end(), part_down.begin(), part_down.end());

    for (size_t i = 1; i < merged.size() - 1; i += 2) {
      comparators.emplace_back(merged[i], merged[i + 1]);
    }
  }
}
```

```cpp
std::pair<std::vector<int>, std::vector<int>> PerepelkinIQsortBatcherOddEvenMergeMPI::Split(
    const std::vector<int> &data) {
  std::vector<int> odd;
  std::vector<int> even;
  for (size_t i = 0; i < data.size(); i++) {
    if (i % 2 == 0) {
      even.push_back(data[i]);
    } else {
      odd.push_back(data[i]);
    }
  }
  return std::make_pair(std::move(odd), std::move(even));
}
```

```cpp
void PerepelkinIQsortBatcherOddEvenMergeMPI::ProcessComparators(
    const std::vector<int> &counts, std::vector<double> &local_data,
    const std::vector<std::pair<int, int>> &comparators) const {
  std::vector<double> peer_buffer;
  std::vector<double> temp;

  for (const auto &comp : comparators) {
    const int first = comp.first;
    const int second = comp.second;

    if (proc_rank_ != first && proc_rank_ != second) {
      continue;
    }

    const int peer = (proc_rank_ == first) ? second : first;
    const int local_size = counts[proc_rank_];
    const int peer_size = counts[peer];

    peer_buffer.resize(peer_size);
    temp.resize(local_size);

    MPI_Status status;
    MPI_Sendrecv(local_data.data(), local_size, MPI_DOUBLE, peer, 0, peer_buffer.data(), peer_size, MPI_DOUBLE, peer, 0,
                 MPI_COMM_WORLD, &status);

    MergeBlocks(local_data, peer_buffer, temp, proc_rank_ == first);
    local_data.swap(temp);
  }
}
```

```cpp
void PerepelkinIQsortBatcherOddEvenMergeMPI::MergeBlocks(const std::vector<double> &local_data,
                                                         const std::vector<double> &peer_buffer,
                                                         std::vector<double> &temp, bool keep_lower) {
  const int local_size = static_cast<int>(local_data.size());
  const int peer_size = static_cast<int>(peer_buffer.size());

  if (keep_lower) {
    for (int tmp_index = 0, res_index = 0, cur_index = 0; tmp_index < local_size; tmp_index++) {
      const double result = local_data[res_index];
      const double current = peer_buffer[cur_index];
      if (result < current) {
        temp[tmp_index] = result;
        res_index++;
      } else {
        temp[tmp_index] = current;
        cur_index++;
      }
    }
  } else {
    for (int tmp_index = local_size - 1, res_index = local_size - 1, cur_index = peer_size - 1; tmp_index >= 0;
         tmp_index--) {
      const double result = local_data[res_index];
      const double current = peer_buffer[cur_index];
      if (result > current) {
        temp[tmp_index] = result;
        res_index--;
      } else {
        temp[tmp_index] = current;
        cur_index--;
      }
    }
  }
}
```

### Приложение №3. Генерация тестовых данных для тестов производительности
```cpp
static std::vector<double> GenerateData(size_t base_length, size_t scale_factor, unsigned int seed) {
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> dist(-1000.0, 1000.0);

    std::vector<double> base(base_length);
    for (double &value : base) {
        value = dist(gen);
    }

    std::vector<double> data;
    data.reserve(base_length * scale_factor);
    for (size_t i = 0; i < scale_factor; ++i) {
        data.insert(data.end(), base.begin(), base.end());
    }
    return data;
}
```
