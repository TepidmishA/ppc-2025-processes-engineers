# Отчёт
# Реализация операции Allreduce с использованием только MPI_Send и MPI_Recv

- **Студент:** Баранов Андрей Александрович, группа: 3823Б1ПР4
- **Технология:** MPI
- **Задача:** Передача от всех одному и рассылка (allreduce)

## 1. Вступление

В параллельных вычислениях, когда процессам надо общаться друг с другом, есть специальные команды. Одна из самых важных – `MPI_Allreduce`.
Она берёт данные со всех процессов, что-то с ними делает (например, складывает) и возвращает итог всем участникам.
В этой работе я пытался повторить принцип работы данной команды, но чтобы она работала только через самые простые способы передачи сообщений: `MPI_Send` и `MPI_Recv`. 
Так же для ускорения работы была использована древовидная структура. Важно было, чтобы моя версия понимала разные типы данных и умела делать разные операции.

## 2. Задача

### 2.1. Формальное определение

Надо написать функцию `CustomAllreduce`. 
Она должна делать то же самое, что и стандартная `MPI_Allreduce`, но без её помощи. Только с помощью `MPI_Send` и `MPI_Recv`.  А так же реализовать передачу данных по дереву для ускорения.

### 2.2. Входные и выходные данные

- **Что даём:**
  - `sendbuf` – буфер отправки данных, у каждого процесса тут свои данные.
  - `count` – колличество эллементов в буфере.
  - `datatype` – какого типа данные (целые, дробные и т.д.).
  - `op` – что с данными делать (например, складывать).
  - `comm` – группа процессов, которые участвуют.
  - `root` – корневой процесс для координации.

- **Что получаем:**
  - `recvbuf` – буфер приема результатов. У всех процессов он должен быть одинаковым.

### 2.3. Какие типы данных и операции поддерживаются

- **Типы данных:** Целые (MPI_INT), дробные (MPI_FLOAT, MPI_DOUBLE)
- **Операции:** Сложение (MPI_SUM)

### 2.4. Примеры

**Пример 1** (2 процесса, главный – процесс 0):
- Процесс 0: [1, 2, 3]
- Процесс 1: [4, 5, 6]
- Итог у всех: [5, 7, 9]

**Пример 2** (3 процесса, главный – процесс 1):
- Процесс 0: [1.0, 2.0]
- Процесс 1: [3.0, 4.0]
- Процесс 2: [5.0, 6.0]
- Итог у всех: [9.0, 12.0]

## 3. Самый простой вариант

В последовательной реализации (SEQ) операция Allreduce тривиальна, так как нет других процессов для коммуникации:

```
bool BaranovACustomAllreduceSEQ::RunImpl() {
  try {
    GetOutput() = GetInput();
    return true;
  } catch (const std::exception &) {
    return false;
  }
}
```

Алгоритм в последовательной версии такой:

1.  Берём данные на входе через `GetInput()`.
2.  Копируем их в выходной буфер.
3.  Выдаём результат через `GetOutput()`.

## 4. Схема распараллеливания алгоритма

### 4.1. Основная идея

Для реализации операции Allreduce используется двухэтапный подход:

1. **Фаза редукции (Reduce)**: все процессы отправляют свои данные корневому процессу, который суммирует их
2. **Фаза широковещательной рассылки (Broadcast)**: корневой процесс рассылает результат всем процессам

### 4.2. Древовидная структура обмена

У нас тут упрощённая схема дерева, где главный процесс – это центр, через который всё проходит:

Все процессы (не корень)
        ↓
Корневой процесс (суммирование)
        ↓
Все процессы (рассылка результата)


### 4.3. Алгоритм CustomAllreduce

**Основной алгоритм в функции** `CustomAllreduce`:

```
void BaranovACustomAllreduceMPI::CustomAllreduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
                                                 MPI_Op op, MPI_Comm comm, int root) {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &size);

  if (count == 0) {
    return;
  }

  int type_size = 0;
  MPI_Type_size(datatype, &type_size);

  std::vector<unsigned char> temp_buf(static_cast<std::size_t>(count) * static_cast<std::size_t>(type_size)); ← //выделение временного буфера
  if (temp_buf.empty()) {
    throw std::runtime_error("Memory allocation failed");
  }

  std::memcpy(temp_buf.data(), sendbuf, static_cast<std::size_t>(count) * static_cast<std::size_t>(type_size));
  //фаза полоучения данных от всех процессов
  if (rank == root) {
    ProcessRootReceive(temp_buf, count, datatype, op, comm, root, size, type_size);
    ProcessRootSend(temp_buf, recvbuf, count, datatype, comm, root, size, type_size);
  } else {
    MPI_Send(sendbuf, count, datatype, root, 0, comm);
    MPI_Recv(recvbuf, count, datatype, root, 1, comm, MPI_STATUS_IGNORE);
  }
}
```
**Алгоритм сбора данных со всех процессов** `ProcessRootReceive`:

```
void ProcessRootReceive(std::vector<unsigned char> &temp_buf, int count, MPI_Datatype datatype, MPI_Op op,
                        MPI_Comm comm, int root, int size, int type_size) {
  for (int i = 0; i < size; i++) {
    if (i != root) {
      std::vector<unsigned char> recv_buf(static_cast<std::size_t>(count) * static_cast<std::size_t>(type_size));
      if (recv_buf.empty()) {
        throw std::runtime_error("Memory allocation failed");
      }

      MPI_Recv(recv_buf.data(), count, datatype, i, 0, comm, MPI_STATUS_IGNORE);
      BaranovACustomAllreduceMPI::PerformOperation(recv_buf.data(), temp_buf.data(), count, datatype, op);
    }
  }
}
```

**Алгоритм отпарвки результата всем процессам** `ProcessRootSend`:

```
void ProcessRootSend(std::vector<unsigned char> &temp_buf, void *recvbuf, int count, MPI_Datatype datatype,
                     MPI_Comm comm, int root, int size, int type_size) {
  for (int i = 0; i < size; i++) {
    if (i != root) {
      MPI_Send(temp_buf.data(), count, datatype, i, 1, comm);
    }
  }
  std::memcpy(recvbuf, temp_buf.data(), static_cast<std::size_t>(count) * static_cast<std::size_t>(type_size));
}
```

**Алгоритм суммирования** `PerformOperation`:

```
void BaranovACustomAllreduceMPI::PerformOperation(void *inbuf, void *inoutbuf, int count, MPI_Datatype datatype,
                                                  MPI_Op op) {
  if (op != MPI_SUM) {
    throw std::runtime_error("Only MPI_SUM operation is supported");
  }

  if (datatype == MPI_INT) {
    auto *in = static_cast<int *>(inbuf);
    auto *inout = static_cast<int *>(inoutbuf);
    for (int i = 0; i < count; i++) {
      inout[i] += in[i];
    }
  } else if (datatype == MPI_FLOAT) {
    auto *in = static_cast<float *>(inbuf);
    auto *inout = static_cast<float *>(inoutbuf);
    for (int i = 0; i < count; i++) {
      inout[i] += in[i];
    }
  } else if (datatype == MPI_DOUBLE) {
    auto *in = static_cast<double *>(inbuf);
    auto *inout = static_cast<double *>(inoutbuf);
    for (int i = 0; i < count; i++) {
      inout[i] += in[i];
    }
  } else {
    throw std::runtime_error("Unsupported datatype");
  }
}
```

### 4.4. Схема коммуникации

```
      Фаза редукции (Reduce)        Фаза рассылки (Broadcast)
              ↑                              ↓             
┌─────────┐      ┌─────────┐      ┌─────────┐
│Процесс 1|-----→│         │-----→│Процесс 1│
├─────────┤      │         │      ├─────────┤
│Процесс 2│-----→│ Корневой│-----→│Процесс 2│
├─────────┤      │ процесс │      ├─────────┤
│Процесс 3│-----→│ (root)  │-----→│Процесс 3│
├─────────┤      │         │      ├─────────┤
│Процесс 4│-----→│         │-----→│Процесс 4│
└─────────┘      └─────────┘      └─────────┘
       |               |               |
       ↓               ↓               ↓
   Данные от      Суммирование     Результат
   процессов         данных        всем процессам
```

### 4.5. Роли процессов

- **Корневой процесс (root)**:
  - Получает данные от всех других процессов
  - Выполняет операцию редукции (суммирование)
  - Рассылает результат всем процессам
  - Хранит финальный результат в своем выходном буфере

- **Не-корневые процессы**:
  - Отправляют свои данные корневому процессу
  - Получают финальный результат от корневого процесса
  - Сохраняют результат в своих выходных буферах

### 4.6. Синхронизация

- Используются разные теги сообщений для фазы редукции (тег 0) и фазы рассылки (тег 1)
- Корневой процесс ожидает данные от всех процессов перед началом рассылки
- Все процессы синхронизированы через получение/отправку сообщений

### 4.7. Код MPI алгоритма

```
bool BaranovACustomAllreduceMPI::RunImpl() {
  try {
    auto input = GetInput();
    auto output = GetOutput();

    if (std::holds_alternative<std::vector<int>>(input)) {
      auto data = std::get<std::vector<int>>(input);
      if (data.empty()) {
        GetOutput() = InTypeVariant{std::vector<int>{}};
        return true;
      }
      auto result_data = std::get<std::vector<int>>(output);
      CustomAllreduce(data.data(), result_data.data(), static_cast<int>(data.size()), MPI_INT, MPI_SUM, MPI_COMM_WORLD,
                      0);

      GetOutput() = InTypeVariant{result_data};
    } else if (std::holds_alternative<std::vector<float>>(input)) {
      auto data = std::get<std::vector<float>>(input);
      if (data.empty()) {
        GetOutput() = InTypeVariant{std::vector<float>{}};
        return true;
      }
      auto result_data = std::get<std::vector<float>>(output);
      CustomAllreduce(data.data(), result_data.data(), static_cast<int>(data.size()), MPI_FLOAT, MPI_SUM,
                      MPI_COMM_WORLD, 0);
      GetOutput() = InTypeVariant{result_data};
    } else if (std::holds_alternative<std::vector<double>>(input)) {
      auto data = std::get<std::vector<double>>(input);
      if (data.empty()) {
        GetOutput() = InTypeVariant{std::vector<double>{}};
        return true;
      }
      auto result_data = std::get<std::vector<double>>(output);
      CustomAllreduce(data.data(), result_data.data(), static_cast<int>(data.size()), MPI_DOUBLE, MPI_SUM,
                      MPI_COMM_WORLD, 0);
      GetOutput() = InTypeVariant{result_data};
    }
    return true;
  } catch (const std::exception &) {
    return false;
  }
}
```

## 5. Детали реализации

### 5.1. Файловая структура

baranov_a_custom_allreduce/
├── common/include/common.hpp
├── mpi/include/ops_mpi.hpp
├── mpi/src/ops_mpi.cpp
├── seq/include/ops_seq.hpp
├── seq/src/ops_seq.cpp
├── tests/functional/main.cpp
├── tests/performance/main.cpp
└── CMakeLists.txt


### 5.2. Ключевые классы и файлы

1. **Общие компоненты (`common`)**:
    - `common.hpp` - общие типы данных и константы, включая `InTypeVariant` для хранения разных типов векторов

2. **Последовательная реализация (SEQ)**:
    - `ops_seq.hpp` - объявление класса `BaranovACustomAllreduceSEQ`
    - `ops_seq.cpp` - реализация методов:
        - `RunImpl()` - основной алгоритм (простое копирование)
        - `ValidationImpl()` - проверка входных данных
        - `PreProcessingImpl()` - подготовка данных
        - `PostProcessingImpl()` - постобработка данных

3. **MPI реализация**:
    - `ops_mpi.hpp` - объявление класса `BaranovACustomAllreduceMPI`
    - `ops_mpi.cpp` - реализация методов:
        - `RunImpl()` - основной алгоритм с вызовом `CustomAllreduce`
        - `CustomAllreduce()` - пользовательская реализация операции Allreduce
        - `PerformOperation()` - выполнение операции редукции для разных типов данных
        - `TreeReduce()` и `TreeBroadcast()` - вспомогательные функции

## 6. Конфигурация системы и инструменты

### 6.1. Системные характеристики

**Системные характеристики**
- **Модель процессора:** AMD Ryzen 5 5600x
- **Количество ядер:** 6 (12 потоков)
- **Тактовая частота:** 3.7 GHz
- **Архитектура:** x64
- **Оперативная память:** 32 GB DDR4 3200 MHz
- **Операционная система:** Windows 10 PRO 64-bit
- **Тип системы:** Настольный компьютер

### 6.2. Набор инструментов

**Компилятор и сборка**
- **Компилятор:** MSVC 2022 (V19.32)
- **Стандарт языка:** C++17
- **Среда разработки:** Visual Studio 2022
- **Тип сборки:** Release
- **Система сборки:** CMake

## 7. Результаты

### 7.1. Методы проверки

- Все тесты проверяют 2 реализации SEQ и MPI
- Результаты тестов SEQ сравниваются с результатами тестов MPI
- Используется фреймворк - Google Test
- Тестирование проводится на 1, 2, 3 и 4 процессах

### 7.2. Результаты функционального тестирования

Все 28 тестов (14 для MPI и 14 для SEQ) прошли успешно для всех конфигураций (1, 2, 3, 4 процесса).
Это подтверждает корректность реализации алгоритма для всех поддерживаемых типов данных и граничных случаев.

**Тестовые случаи:**
1. `positive_doubles` - положительные числа типа double
2. `mixed_doubles` - смешанные положительные и отрицательные числа
3. `negative_doubles` - отрицательные числа
4. `large_doubles` - большие числа
5. `fractional_doubles` - дробные числа
6. `empty_vector` - пустой вектор
7. `single_element` - вектор из одного элемента
8. `zeros` - вектор из нулей
9. `integers` - целые числа
10. `floats` - числа с плавающей точкой
11. `float_with_suffix `- числа с плавающей точкой с суффиксом F (например, 1.1F, 2.2F)
12. `nan_test` - тест с NaN значениями
13. `inf_test` - тест с бесконечностями
14. `large_count_test` - тест с большим количеством элементов

### 7.3. Производительность

**Результаты измерения производительности для вектора длиной 10,000,000 элементов типа double:**

**Время выполнения (task_run) - чистые вычисления**

| Режим | Процессы | Время, с | Ускорение | Эффективность |
|-------|----------|----------|-----------|---------------|
| seq   | 1        | 0.005320 | 1.00      | 100.0%        |
| mpi   | 1        | 0.158554 | 0.034     | 3.4%          |
| mpi   | 2        | 0.288949 | 0.018     | 0.9%          |
| mpi   | 3        | 0.422419 | 0.013     | 0.4%          |
| mpi   | 4        | 0.570020 | 0.009     | 0.2%          |

**Время выполнения (pipeline) - полный цикл**

| Режим | Процессы | Время, с | Ускорение | Эффективность |
|-------|----------|----------|-----------|---------------|
| seq   | 1        | 0.005390 | 1.00      | 100.0%        |
| mpi   | 1        | 0.158560 | 0.034     | 3.4%          |
| mpi   | 2        | 0.291478 | 0.018     | 0.9%          |
| mpi   | 3        | 0.448011 | 0.012     | 0.4%          |
| mpi   | 4        | 0.562088 | 0.010     | 0.2%          |

## 8. Выводы

### 8.1. Достижения

**Корректность реализации:**
- Реализована пользовательская версия операции `Allreduce` с использованием только `MPI_Send` и `MPI_Recv`
- Поддерживаются три типа данных: MPI_INT, MPI_FLOAT, MPI_DOUBLE
- Реализована операция MPI_SUM для всех поддерживаемых типов
- Все функциональные тесты пройдены успешно
- Обработаны граничные случаи (пустые векторы, NaN, бесконечности)

**Архитектурные решения:**
- Использован вариант (variant) для хранения разных типов данных
- Реализована модульная архитектура с разделением на SEQ и MPI версии
- Применена древовидная схема коммуникации (хотя и упрощенная)

### 8.2. Ограничения и проблемы

**Производительность:**
- Централизованная схема коммуникации создает bottleneck на корневом процессе
- Значительные накладные расходы на копирование данных и синхронизацию
- Алгоритм не масштабируется с увеличением числа процессов
- Последовательная версия значительно быстрее MPI версии

**Функциональность:**
- Поддерживается только операция MPI_SUM
- Не реализованы более сложные древовидные схемы коммуникации
- Ограниченная обработка ошибок

### 8.3. Заключение

Реализация успешно демонстрирует принцип работы операции `Allreduce` с использованием только базовых функций передачи сообщений. 
Хотя производительность реализации оставляет желать лучшего из-за централизованной схемы коммуникации, корректность алгоритма подтверждена всесторонним тестированием. 
Работа может служить основой для более оптимизированных реализаций с улучшенными схемами коммуникации.
