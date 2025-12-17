# Минимальное значение элементов вектора

- Студент: Синев Артём Александрович, группа 3823Б1ПР2
- Технологии: SEQ, MPI
- Вариант: 4

## 1. Введение

Задача поиска минимального элемента в векторе является одной из фундаментальных операций в вычислительной математике и анализе данных. Данная работа посвящена разработке и реализации параллельного алгоритма для поиска минимального элемента в векторе с использованием технологии MPI (Message Passing Interface). Алгоритм обеспечивает эффективное распределение вычислений между процессами и позволяет масштабировать решение на большое количество вычислительных узлов.

## 2. Постановка задачи

Дан вектор целых чисел. Требуется найти минимальный элемент.

**Входные данные:**
- vector - одномерный вектор целых чисел произвольной длины N

**Выходные данные:**
- min_value - целое число, содержащее минимальное значение в векторе

**Ограничения:**
- Вектор должен содержать хотя бы один элемент
- Элементы вектора - целые числа (int)
- Допускаются отрицательные значения

**Пример:**

Входные данные: vector = [5, 3, 8, 1, 9, 2]
Выходные данные: min_value = 1

## 3. Описание базового алгоритма (последовательная версия)

Последовательный алгоритм поиска минимального элемента в векторе выполняет линейный проход по всем элементам с обновлением текущего минимального значения.

Алгоритм состоит из следующих шагов:

1. **Инициализация**: Установка начального минимального значения как первого элемента вектора
2. **Обработка строк**: Для каждого элемента от 1 до N-1::
   - Сравнение текущего элемента с сохраненным минимальным значением
   - Обновление минимального значения при обнаружении меньшего элемента
3. **Завершение**: Возврат найденного минимального значения

**Сложность алгоритма**

**Время:** O(N)
- Один проход по всем элементам вектора
- N-1 операций сравнения

**Память:** O(N)
- O(N) для хранения входного вектора
- O(1) для временных переменных
- **Итого:** O(N)

## 4. Схема распараллеливания

Параллельная реализация основана на разделении вектора на блоки между доступными процессами с использованием стратегии блочного распределения

```cpp
block_size = vector_size / proc_count      // Базовый размер блока
remainder = vector_size % proc_count       // Остаточные элементы
```

**Распределение работы**

- Исходный вектор хранится только на процессе с рангом 0
- Данные распределяются между процессами с помощью MPI_Scatterv
- Каждый процесс получает непрерывный блок элементов
- Процессы с меньшим rank получают дополнительный элемент при наличии остатка

**Роли процессов**
- **Процесс 0:** Хранит исходные данные, распределяет данные между процессами
- **Все процессы:** Выполняют вычисления на выделенных порциях данных
- **MPI_Allreduce:** Собирает локальные минимумы и вычисляет глобальный минимум

**Схема коммуникации**
- **Фаза вычислений:** Каждый процесс независимо вычисляет локальный минимум
- **Фаза редукции:** MPI_Allreduce собирает все локальные минимумы и находит глобальный минимум
- **Фаза синхронизации:** MPI_Bcast для распространения размера данных
- **Фаза распределения:** MPI_Scatterv распределяет данные между процессами

## 5. Детали реализации

**Файлы:**
- `common/include/common.hpp` - определение типов данных
- `seq/include/ops_seq.hpp`, `seq/src/ops_seq.cpp` - последовательная реализация
- `mpi/include/ops_mpi.hpp`, `mpi/src/ops_mpi.cpp` - параллельная реализация
- `tests/functional/main.cpp` - функциоанльные тесты
- `tests/performance/main.cpp` - тесты производительности

**Ключевые классы:**
- `SinevAMinInVectorSEQ` - последовательная реализация
- `SinevAMinInVectorMPI` - параллельная MPI реализация

**Основные методы:**
- `ValidationImpl()` - проверка входных данных
- `PreProcessingImpl()` - подготовительные вычисления  
- `RunImpl()` - основной алгоритм
- `PostProcessingImpl()` - завершающая обработка

**Ключевые особенности реализации:**
```cpp
bool SinevAMinInVectorMPI::ValidationImpl() {
  int proc_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

  bool is_valid = true;

  // Проверка выполняется только на root процессе
  if (proc_rank == 0) {
    is_valid = !GetInput().empty();
  }

  // Результат проверки рассылается всем процессам
  MPI_Bcast(&is_valid, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);

  return is_valid;
}

bool SinevAMinInVectorMPI::RunImpl() {
  int proc_num = 0;
  int proc_rank = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &proc_num);
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

  std::vector<int> local_data;
  int global_size = 0;

  // Только процесс 0 знает исходные данные
  if (proc_rank == 0) {
    global_size = static_cast<int>(GetInput().size());
  }

  // Рассылаем размер всем процессам
  MPI_Bcast(&global_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (global_size == 0) {
    GetOutput() = std::numeric_limits<int>::max();
    return true;
  }

  // Вычисляем распределение данных
  int block_size = global_size / proc_num;
  int remainder = global_size % proc_num;

  // Каждый процесс вычисляет свой локальный размер
  int local_size = block_size + (proc_rank < remainder ? 1 : 0);
  local_data.resize(local_size);

  // Подготавливаем массивы для Scatterv
  std::vector<int> sendcounts(proc_num);
  std::vector<int> displacements(proc_num);

  // Только процесс 0 заполняет массивы распределения
  if (proc_rank == 0) {
    for (int i = 0; i < proc_num; i++) {
      sendcounts[i] = block_size + (i < remainder ? 1 : 0);
      displacements[i] = (i * block_size) + std::min(i, remainder);
    }
  }

  // Рассылаем информацию о распределении всем процессам
  MPI_Bcast(sendcounts.data(), proc_num, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(displacements.data(), proc_num, MPI_INT, 0, MPI_COMM_WORLD);

  // Распределяем данные по процессам
  MPI_Scatterv(proc_rank == 0 ? GetInput().data() : nullptr, 
               sendcounts.data(), displacements.data(), MPI_INT,
               local_data.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);

  // Локальное вычисление минимума
  int local_min = std::numeric_limits<int>::max();
  for (int value : local_data) {
    local_min = std::min(local_min, value);
  }

  // Глобальная редукция для нахождения общего минимума
  int global_min = std::numeric_limits<int>::max();
  MPI_Allreduce(&local_min, &global_min, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);

  GetOutput() = global_min;
  return true;
}

bool SinevAMinInVectorMPI::PostProcessingImpl() {
  return true;
}
```
## Особенности обработки граничных случаев

В процессе разработки возникла проблема с валидацией результатов в методе PostProcessingImpl(). Изначальная реализация содержала проверку:

```cpp
// НЕВЕРНАЯ реализация
bool PostProcessingImpl() {
  return GetOutput() > INT_MIN;
}
```
Данная проверка некорректно отвергала валидные результаты, когда минимальным элементом вектора действительно являлось значение INT_MIN.
Было изменено на:

```cpp
bool PostProcessingImpl() {
  return true;  // Если алгоритм дошел до этой стадии - все корректно
}
```

**Был выбран подход безусловного успеха, так как:**
  - Корректность алгоритма гарантируется на этапах Validation и Run
  - Минимальные накладные расходы - отсутствие дополнительных проверок

В финальной версии также произведен переход с макроса INT_MIN на типобезопасный `std::numeric_limits<int>::min()`:

```cpp
// Более безопасная альтернатива
GetOutput() = std::numeric_limits<int>::max();  // Вместо INT_MAX
```


## 6. Экспериментальное окружение

### 6.1 Аппаратное обеспечение/ОС:

- **Процессор:** Intel Core i7-13700HX
- **Ядра:** 16 физических ядер  
- **ОЗУ:** 8 ГБ 
- **ОС:** Kubuntu 24.04

### 6.2 Программный инструментарий

- **Компилятор:** g++ 13.3.0
- **Тип сборки:** Release
- **Стандарт C++:** C++20
- **MPI:** OpenMPI 4.1.6

### 6.3 Тестовое окружение

```bash
PPC_NUM_PROC=1,2,4
```

## 7. Результаты

### 7.1. Корректность работы

Все функциональные тесты пройдены успешно:
- mixed_positive: Вектор со смешанными положительными числами [5, 3, 8, 1, 9, 2]
- with_negatives: Вектор с отрицательными числами [10, -5, 7, 0, 15] 
- single_element: Вектор из одного элемента [42] 
- zero_only: Вектор с нулевым элементом [0]
- all_negative: Вектор только с отрицательными числами [-10, -140, -45, -24, -99]

SEQ и MPI версии выдают идентичные результаты для всех тестовых случаев.

### 7.2. Производительность

**Время выполнения (секунды) для матрицы 5000×5000:**

| Версия | Количество процессов | Task Run время |
|--------|---------------------|----------------|
| SEQ    | 1                   | 0.3933         |
| MPI    | 1                   | 0.5537         | 
| MPI    | 2                   | 0.2972         |
| MPI    | 4                   | 0.1748         |

**Ускорение относительно SEQ версии:**

| Количество процессов | Ускорение | Эффективность |
|---------------------|-----------|---------------|
| 1                   | 0.71×     | 71%           |
| 2                   | 1.32×     | 66%           |
| 4                   | 2.25×     | 56%           |


**Формула ускорения:** Ускорение = Время SEQ / Время MPI

**Формула эффективности:** Эффективность = (Ускорение / Количество процессов) × 100%

### 7.3. Анализ эффективности

- **Лучшее ускорение:** 2.25× на 4 процессах
- **Оптимальная конфигурация:** 4 процесса
- **Эффективность MPI:** Снижается с увеличением количества процессов

### 7.4. Наблюдения

1. **MPI с 1 процессом** показывает меньшую производительность из-за накладных расходов MPI
2. **MPI с 2 процессами** демонстрирует ускорение 1.32×
3. **MPI с 4 процессами** достигает максимального ускорения 2.25×
4. **Снижение эффективности** связано с затратами на распределение данных

## 8. Выводы

### 8.1. Достигнутые результаты

- **Корректность:** Успешное прохождение всех функциональных тестов
- **Эффективность параллелизации:** Ускорение 2.25× на 4 процессах
- **Масштабируемость:**  Хорошее масштабирование с увеличением процессов
- **Надежность:** Распределенный подход для больших данных

### 8.2. Особенности реализации

- **Распределенный подход:** Использование MPI_Scatterv для распределения данных
- **Синхронизация:** MPI_Bcast для согласованной проверки входных данных
- **Балансировка нагрузки:** Равномерное распределение с учетом остатка
- **СОптимизация коммуникаций:** Минимизация дублирования данных

### 8.3. Ограничения и перспективы
- **Накладные расходы:** Коммуникационные затраты для малых данных
- **Оптимальность:** Наиболее эффективен для объемных данных
- **Перспективы:** Неблокирующие операции для перекрытия вычислений

## 9. Источники
1. Лекции по параллельному программированию Сысоева А. В
2. Документация MPI: https://www.open-mpi.org/
3. Материалы курса: https://github.com/learning-process/ppc-2025-processes-engineers

## 10. Приложение

```cpp
#include "sinev_a_min_in_vector/mpi/include/ops_mpi.hpp"
#include <mpi.h>
#include <algorithm>
#include <limits>
#include <vector>

#include "sinev_a_min_in_vector/common/include/common.hpp"

namespace sinev_a_min_in_vector {

SinevAMinInVectorMPI::SinevAMinInVectorMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::numeric_limits<int>::max();
}

bool SinevAMinInVectorMPI::ValidationImpl() {
  int proc_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

  bool is_valid = true;

  if (proc_rank == 0) {
    is_valid = !GetInput().empty();
  }

  MPI_Bcast(&is_valid, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);

  return is_valid;
}

bool SinevAMinInVectorMPI::PreProcessingImpl() {
  return true;
}

bool SinevAMinInVectorMPI::RunImpl() {
  int proc_num = 0;
  int proc_rank = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &proc_num);
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

  std::vector<int> local_data;
  int global_size = 0;

  if (proc_rank == 0) {
    global_size = static_cast<int>(GetInput().size());
  }

  // Рассылаем размер всем процессам
  MPI_Bcast(&global_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (global_size == 0) {
    GetOutput() = std::numeric_limits<int>::max();
    return true;
  }

  int block_size = global_size / proc_num;
  int remainder = global_size % proc_num;

  int local_size = block_size + (proc_rank < remainder ? 1 : 0);
  local_data.resize(local_size);

  std::vector<int> sendcounts(proc_num);
  std::vector<int> displacements(proc_num);

  if (proc_rank == 0) {
    for (int i = 0; i < proc_num; i++) {
      sendcounts[i] = block_size + (i < remainder ? 1 : 0);
      displacements[i] = (i * block_size) + std::min(i, remainder);
    }
  }

  MPI_Bcast(sendcounts.data(), proc_num, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(displacements.data(), proc_num, MPI_INT, 0, MPI_COMM_WORLD);

  MPI_Scatterv(proc_rank == 0 ? GetInput().data() : nullptr, sendcounts.data(), displacements.data(), MPI_INT,
               local_data.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);

  int local_min = std::numeric_limits<int>::max();
  for (int value : local_data) {
    local_min = std::min(local_min, value);
  }

  int global_min = std::numeric_limits<int>::max();
  MPI_Allreduce(&local_min, &global_min, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);

  GetOutput() = global_min;
  return true;
}

bool SinevAMinInVectorMPI::PostProcessingImpl() {
  return true;
}

}  // namespace sinev_a_min_in_vector

```