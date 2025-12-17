# Сортировка пузырьком (алгоритм чет-нечетной перестановки)

- Student: Овчинников Матвей Евгеньевич, group 3823Б1ПР2
- Technology: SEQ | MPI
- Variant: 21

## 1. Introduction

- **Brief motivation:** Изучить принципы параллельных вычислений, освоить технологии MPI, реализовать алгоритм сортировки (odd-even) и погрузиться в общение процессов между собой ещё глубже.
- **Problem context:** Сравнить производительности последовательной и параллельной реализации алгоритма сортировки odd-even.
- **Expected outcome:** Получить навыки работы с параллельными процессами, изучить внутренности MPI и как устроена архитектура параллелизма.

## 2. Problem Statement

- **Formal task definition:** Используя средства Open MPI, нужно написать последовательную и параллельную версии программы, которые позволят найти максимальное значение в каждом столбце введенной матрицы. Сравнить скорости работы полученных реализаций через perfomance тесты, а так же проверить их валидность посредством functional тестов.
- **input/output format:** На вход подается вектор целых чисел (std::vector<int>), а на выходе отсортированный вектор такого же формата.

## 3. Baseline Algorithm (Sequential)

Если на вход подаётся пустой вектор, то функция вернёт значение true, иначе начинается алгоритм пузырьковой сортировки odd-even.

В алгоритме есть две фазы: четная, на которой мы проходимся по всему вектору и сравниваем пары элементов, начиная с нуля (т.е берём 0 и 1, 2 и 3, ...). Нечетная фаза аналогичная, но там индексация начинается с единицы (т.е берём 1 и 2, 3 и 4, ...). Если левый элемент оказался больше правого, то мы меняем их местами (я использовал std::swap()). Фазы повторяются до тех пор, пока вектор не отсортируется, это реализовано при помощи переменной sorted, которая управляет внешним циклом while. В начале каждой итерации она ставится в положение true, что предполагает завершение цикла в конце итерации, но если хотя бы в одной из фаз мы поменяем элементы хотя бы одной пары, то флаг опускается и внешний цикл продолжает работать.

```cpp
bool OvchinnikovMBubbleSortSEQ::RunImpl() {
  auto &array = GetInput();
  if (array.empty()) {
    return true;
  }
  bool sorted = false;
  while (!sorted) {
    sorted = true;
    for (size_t i = 0; i < array.size() - 1; i += 2) {
      if (array[i] > array[i + 1]) {
        std::swap(array[i], array[i + 1]);
        sorted = false;
      }
    }
    for (size_t i = 1; i < array.size() - 1; i += 2) {
      if (array[i] > array[i + 1]) {
        std::swap(array[i], array[i + 1]);
        sorted = false;
      }
    }
  }
  GetOutput() = array;
  return true;
}
```


## 4. Parallelization Scheme

- **Data distribution:**
Данные делятся нулевым процессом между остальными следующим образом:
1. сначала высчитывается базовый размер кусочка, который достанется каждому процессу. (Мы делим размер вектора на кол-во процессов)
2. после этого есть шанс, что кол-во процессов не кратно размеру вектора, следовательно нужно достать остаток вектора, который мы потеряли, для этого берем остаток от деления размера вектора на процессы.
3. Чтобы i-ый процесс знал, где он должен работать, ему нужно знать где начальная граница его блока данных, для этого переступим через все блоки процессов, которые предшествуют процессу i и прибавим минимум из номера процесса и остатка размера вектора.
4. Для понимания конца своего блока достаточно прибавить к началу размер блока и в зависимости от того, меньше ли номер процесса, чем остаток, мы прибавляем единицу или ноль соответственно.

Этот алгоритм широко известен в HPC-литературе под несколькими именами, но его суть в том, чтобы распределить данные между процессами как можно равномернее, то есть по итогу его работы любые два блока данных не отличаются размерами больше, чем на единицу.

- **Rank roles:**
  - **Process 0:** 
  1. Распределяет данные:
  ```cpp
    if (rank == 0) {
    int chunk_base = vec_size / proc_count;
    int chunk_extra = vec_size % proc_count;
    int offset = 0;

    for (int i = 0; i < proc_count; i++) {
      int chunk_end_bit = static_cast<int>(i < chunk_extra);
      elem_count[i] = chunk_base + chunk_end_bit;
      elem_offset[i] = offset;
      offset += elem_count[i];
    }
  }
  ```

  2. Определяет размер итогового вектора:
  ```cpp
    if (rank == 0) {
    result.resize(vec_size);
  }
  ```

  3. Переносит результат в GetOutput():
  ```cpp
  if (rank == 0) {
    GetOutput() = result;
  }
  ```

  - **All processes:** 
  1. Используют функцию последовательной сортировки для локальной обработки своего участка данных:
```cpp
void SortOddEven(std::vector<int> &array) {
  if (array.empty()) {
    return;
  }
  bool sorted = false;
  while (!sorted) {
    sorted = true;
    for (size_t i = 0; i + 1 < array.size(); i += 2) {
      if (array[i] > array[i + 1]) {
        std::swap(array[i], array[i + 1]);
        sorted = false;
      }
    }
    for (size_t i = 1; i + 1 < array.size(); i += 2) {
      if (array[i] > array[i + 1]) {
        std::swap(array[i], array[i + 1]);
        sorted = false;
      }
    }
  }
}
```
2. А так же функцию для управления параллельной сортировкой:
```cpp
void ManageOddEven(int rank, int proc_count, std::vector<int> &local_data) {
  for (int phase = 0; phase < (proc_count + 1); phase++) {
    int partner = FindPartner(rank, phase);

    if (partner >= 0 && partner < proc_count) {  // partner validation
      std::vector<int> partner_data = SwapDataWithPartner(partner, local_data);
      local_data = MergeAndSplit(rank, partner, local_data, partner_data);
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);
}
```



## 5. Implementation Details

- **Code structure (files, key classes/functions):**
    функции ValidationImpl, PreProcessingImpl, PostProcessingImpl не были использованы мною - всегда возвращается true. Вся логика содержится в функции:
```cpp
bool OvchinnikovMBubbleSortMPI::RunImpl() {
  int rank = 0;
  int proc_count = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &proc_count);

  const auto &input_vector = GetInput();
  int vec_size = static_cast<int>(input_vector.size());

  if (vec_size == 0) {
    GetOutput() = std::vector<int>();
    return true;
  }

  std::vector<int> elem_count(proc_count);
  std::vector<int> elem_offset(proc_count);

  if (rank == 0) {
    int chunk_base = vec_size / proc_count;
    int chunk_extra = vec_size % proc_count;
    int offset = 0;

    for (int i = 0; i < proc_count; i++) {
      int chunk_end_bit = static_cast<int>(i < chunk_extra);
      elem_count[i] = chunk_base + chunk_end_bit;
      elem_offset[i] = offset;
      offset += elem_count[i];
    }
  }

  MPI_Bcast(elem_count.data(), proc_count, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(elem_offset.data(), proc_count, MPI_INT, 0, MPI_COMM_WORLD);

  std::vector<int> local_data(elem_count[rank]);
  MPI_Scatterv(rank == 0 ? input_vector.data() : nullptr, elem_count.data(), elem_offset.data(), MPI_INT,
               local_data.data(), elem_count[rank], MPI_INT, 0, MPI_COMM_WORLD);

  SortOddEven(local_data);
  ManageOddEven(rank, proc_count, local_data);
  std::vector<int> result;
  if (rank == 0) {
    result.resize(vec_size);
  }

  MPI_Gatherv(local_data.data(), static_cast<int>(local_data.size()), MPI_INT, rank == 0 ? result.data() : nullptr,
              elem_count.data(), elem_offset.data(), MPI_INT, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    GetOutput() = result;
  } else {
    GetOutput().resize(vec_size);
  }

  MPI_Bcast(GetOutput().data(), vec_size, MPI_INT, 0, MPI_COMM_WORLD);
  return true;
}
```


# ManageOddEven() была декомпозирована и разбита на 3 небольшие функции:
1. Функция по поиску партнера для процесса rank:
```cpp
int FindPartner(int rank, int phase) {
  int partner = 0;
  if (phase % 2 == 0) {
    if (rank % 2 == 0) {
      partner = rank + 1;
    } else {
      partner = rank - 1;
    }
  } else {
    if (rank % 2 == 0) {
      partner = rank - 1;
    } else {
      partner = rank + 1;
    }
  }
  return partner;
}
```
В зависимости от текущей фазы сортировки и номера процесса ему подбирается левый или правый партнер.
2. Функция для обмена данными между процессами:
```cpp
std::vector<int> SwapDataWithPartner(int partner, const std::vector<int> &local_data) {
  // Exchange sizes first
  int my_size = static_cast<int>(local_data.size());
  int partner_size = 0;

  MPI_Sendrecv(&my_size, 1, MPI_INT, partner, 0, &partner_size, 1, MPI_INT, partner, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);

  // Exchange data
  std::vector<int> partner_data(partner_size);
  MPI_Sendrecv(local_data.data(), my_size, MPI_INT, partner, 1, partner_data.data(), partner_size, MPI_INT, partner, 1,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  return partner_data;
}
```
На вход подаются оригинальные данные текущего процесса и номер процесса-партнёра, с которым будет произведён обмен. Сначала процессы меняются размерами блоков данных, а затем и самим блоком.
Общение осуществлено через MPI_Sendrecv(). Эта функция позволяет процессу отправить и принять данные одновременно.
3. Функция для перемещения чисел между двумя процессами:
```cpp
std::vector<int> MergeAndSplit(int rank, int partner, const std::vector<int> &local_data,
                               const std::vector<int> &partner_data) {
  std::vector<int> merged;
  merged.reserve(local_data.size() + partner_data.size());
  size_t i = 0;
  size_t j = 0;
  while (i < local_data.size() && j < partner_data.size()) {
    if (local_data[i] <= partner_data[j]) {
      merged.push_back(local_data[i]);
      i++;
    } else {
      merged.push_back(partner_data[j]);
      j++;
    }
  }
  while (i < local_data.size()) {
    merged.push_back(local_data[i]);
    i++;
  }
  while (j < partner_data.size()) {
    merged.push_back(partner_data[j]);
    j++;
  }
  // std::ranges::merge(local_data.begin(), local_data.end(), partner_data.begin(), partner_data.end(), merged.begin());
  // решил использовать свою реализацию merge
  std::vector<int> result(local_data.size());
  if (rank < partner) {
    // Keep smaller half
    std::copy(merged.begin(), merged.begin() + static_cast<int>(local_data.size()), result.begin());
  } else {
    // Keep larger half
    std::copy(merged.end() - static_cast<int>(local_data.size()), merged.end(), result.begin());
  }

  return result;
}
```
На вход подаются номера двух процессов и их блоки данных. Важно, что вектора процессов предварительно были локально отсортированы алгоритмом odd-even, потому что далее они будут сливаться в один общий вектор merged, который в итоге тоже должен быть отсортирован. Сортировка общего вектора реализована путём правильного слияния двух входных массивов (в процессе заполнения общего вектора сравниваются элемент первого и второго вектора, таким образом определяется какой элемент будет добавлен в вектор merged следующим). Для подобного рода слияния обязательна предварительная сортировка входных векторов.
После слияния у нас имеется один большой отсортированный массив чисел, который нужно правильно поделить между двумя процессами, это достигается сравнением номеров процессов (меньшему процессу достаётся первая часть вектора с меньшими элементами, а большему процессу выдаётся вторая часть с большими значениями).


- Important assumptions and corner cases: если на вход подаётся пустой вектор, то возвращается true

- Memory usage considerations: исходные данные доступны исключительно нулевому процессу, который распределяет их и отправляет блоками остальным процессам.

## 6. Experimental Setup

- Hardware/OS: CPU model, cores/threads, RAM, OS version:

- Модель ЦП: 13th Gen Intel(R) Core(TM) i7-13700H

- Архитектура: x86_64

- Ядра/потоки: 6 ядер, 12 потоков

- ОЗУ: 16 ГБ

- Версия ОС: WSL: 2.6.1.0

- Ядро: Linux 6.8.0-87-generic

- Toolchain:

- GCC 13.1.0 (Ubuntu 13.1.0-8ubuntu1~22.04)

- Clang 18.1.3

- CMake 3.28.3

- GNU Make 4.3

## 7. Results and Discussion

### 7.1 Correctness

Корректность работы была проверена с помощью комплексного модульного тестирования с использованием фреймворка Google Test. Набор тестов включает в себя 6 тестовых случаев, охватывающих различные сценарии:

```cpp
const std::array<TestType, 7> kTestParams = {
    TestType{std::vector<int>{}, "empty"},
    TestType{std::vector<int>{5}, "one_elem"},
    TestType{std::vector<int>{6, 7, 35, 2, 3}, "random_5"},
    TestType{std::vector<int>{1, 2, 3, 4, 5}, "sorted"},
    TestType{std::vector<int>{-1, 100, 0, -5, 20}, "negative"},
    TestType{std::vector<int>{7, 7, 7, 7}, "same_numbers"},
    TestType{std::vector<int>{9, 1, 5, 3, 8, 2, 7, 4, 6, 0}, "random_10"},
};
```

Все тесты пройдены успешно.

### 7.2 Performance

Present time, speedup and efficiency. Example table:

| Mode | Count | Time, s       | Speedup | Efficiency |
|------|-------|---------------|---------|------------|
| seq  | 1     | 0.46 ± 0.02   | 1.00    | N/A     |
| seq  | 1     | 0.47 ± 0.03   | 1.00    | N/A     |
| mpi  | 2     | 0.36 ± 0.01   | 1.29    | 64.6%      |
| mpi  | 2     | 0.37 ± 0.03   | 1.26    | 62.8%      |
| mpi  | 3     | 0.17 ± 0.01   | 2.74    | 91.2%      |
| mpi  | 3     | 0.17 ± 0.005  | 2.74    | 91.2%      |
| mpi  | 4     | 0.10 ± 0.01   | 4.65    | 116.2%     |
| mpi  | 4     | 0.10 ± 0.01   | 4.65    | 116.2%     |
| mpi  | 6     | 0.06 ± 0.005  | 7.75    | 129.2%     |
| mpi  | 6     | 0.075 ± 0.005 | 6.20    | 103.3%     |
| mpi  | 8     | 0.04 ± 0.005  | 11.63   | 145.3%     |
| mpi  | 8     | 0.04 ± 0.006  | 11.63   | 145.3%     |

Для чистоты эксперимента я запускал по 5 раз перфоманс тесты для каждого количества процессов, я брал минимальное и максимальное получившееся значение, находил среднее и подписал погрешность тестирования.

Объем данных составлял 100.000 элементов в векторе.

Результаты впечатляющие, уже на 3-х процессах программа показывает 90% эффективности, а на 4 процессах превышает теоретический потолок возможного ускорения (ускорение на 4 процессах не может быть больше, чем в 4 раза, а у меня оно в 4.5 раз). При увеличении кол-ва процессов этот парадокс только усиливается и наблюдается эффективность в 145% при 8-ми процессах, параллельно сортирующих массив из 100 тысяч чисел. Изучив вопрос в открытых источниках, я вижу две причины такого "суперлинейного" ускорения. Во-первых, метод сортировки odd-even имеет сложность O(n^2), а значит при уменьшении n локальная сортировка даёт квадратичный выигрыш. Во-вторых, чем меньше размер массива, тем лучше он помещается в кэш процессора, а значит доступ к памяти осуществляется быстрее.

## 8. Conclusions

Summarize findings and limitations.

Успешно были реализованы MPI и SEQ версии алгоритма пузырьковой сортировки (odd-even). Параллельная версия демонстрирует значительное преимущество в производительности перед последовательной, иногда выходя за рамки возможностей. 

В этой работе я столкнулся с проблемой сложности главной функции, что заставило меня упрощать её: убирать множественные вложенности, выделять участки кода и выносить их в отдельные функции, а также применять смекалку для особых случаев, например эта строка кода позволила избавиться от конструкции if/else:
```cpp
int chunk_end_bit = static_cast<int>(i < chunk_extra);
```
Помимо декомпозиции, пришлось углубиться в общение процессов между собой, я изучил такие функции библиотеки MPI как:
1. MPI_Sendrecv();
2. MPI_Gatherv(); 

## 9. Ссылки

1. Документация Open MPI
2. Учебные материалы по параллельному программированию
3. Записи онлайн лекций Сысоева Александра Владимировича