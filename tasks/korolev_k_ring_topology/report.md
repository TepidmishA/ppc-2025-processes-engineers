# Топология "Кольцо" — Королёв Кузьма

- Student: Королёв Кузьма, group 3823Б1ПР3  
- Technology: SEQ | MPI  
- Variant: Топология "Кольцо"  

## 1. Introduction
Разработка однопоточного (SEQ) и параллельного (MPI) алгоритмов передачи данных по топологии "Кольцо".  
Цель – реализовать передачу данных от любого выбранного процесса любому другому процессу, используя возможности MPI по работе с коммуникаторами, но без использования `MPI_Cart_Create` и `MPI_Graph_Create`.

## 2. Problem Statement
Необходимо реализовать топологию сети передачи данных "Кольцо", обеспечив возможность передачи данных между любыми двумя процессами.

**Вход:** структура `RingMessage`, содержащая:
- `source` — номер процесса-отправителя
- `dest` — номер процесса-получателя
- `data` — вектор целых чисел для передачи

**Выход:** вектор целых чисел — полученные данные.

**Ограничения:**
- Запрещено использовать `MPI_Cart_Create` и `MPI_Graph_Create`
- Передача данных должна осуществляться только между соседними процессами

Примеры:
- Процесс 0 → Процесс 3 (при 4 процессах): данные проходят через процессы 1 и 2
- Процесс 3 → Процесс 0 (при 4 процессах): данные передаются напрямую соседу

## 3. Baseline Algorithm (Sequential)

Последовательная реализация эмулирует передачу данных по кольцу путём многократного копирования данных между буферами.

```
current_data = input.data
temp_buffer = empty vector of same size

for iter = 0 to num_iterations:
    for i = 0 to data_size:
        temp_buffer[i] = current_data[i]
    // Эмуляция обработки на процессе
    for i = 0 to data_size:
        temp_buffer[i] += iter
        temp_buffer[i] -= iter
    swap(current_data, temp_buffer)

output = current_data
```

Алгоритм работает за O(N * K), где N — размер данных, K — количество итераций эмуляции.

## 4. Parallelization Scheme

### Топология "Кольцо"

В топологии "Кольцо" каждый процесс имеет ровно двух соседей:
- **Левый сосед:** `(rank - 1 + size) % size`
- **Правый сосед:** `(rank + 1) % size`

```
    P0 ──→ P1
    ↑       ↓
    P3 ←── P2
```

### Алгоритм передачи

Направление передачи: всегда по часовой стрелке (вправо).

```
steps_right = (dest - source + size) % size

if rank == source:
    MPI_Send(data_size, right_neighbor)
    MPI_Send(data, right_neighbor)

current_step = (rank - source + size) % size

if current_step > 0 and current_step <= steps_right:
    MPI_Recv(data_size, left_neighbor)
    MPI_Recv(data, left_neighbor)
    
    if rank == dest:
        save data to output
    else:
        MPI_Send(data_size, right_neighbor)
        MPI_Send(data, right_neighbor)

MPI_Bcast(result from dest to all processes)
```

### Коммуникация
- Использование `MPI_Send`/`MPI_Recv` между соседними процессами
- `MPI_Bcast` для распространения результата на все процессы
- Роли рангов определяются динамически на основе `source` и `dest`

## 5. Implementation Details

Структура кода:

```
tasks/korolev_k_ring_topology/
├── common
│   └── include
│       └── common.hpp       # RingMessage, InType, OutType
├── info.json
├── mpi
│   ├── include
│   │   └── ops_mpi.hpp      # KorolevKRingTopologyMPI
│   └── src
│       └── ops_mpi.cpp      # Реализация передачи по кольцу
├── report.md
├── seq
│   ├── include
│   │   └── ops_seq.hpp      # KorolevKRingTopologySEQ
│   └── src
│       └── ops_seq.cpp      # Эмуляция передачи
├── settings.json
└── tests
    ├── .clang-tidy
    ├── functional
    │   └── main.cpp         # 8 функциональных тестов
    └── performance
        └── main.cpp         # Тесты производительности
```

Ключевые особенности реализации MPI:
- Передача данных строго через соседей (left/right neighbor)
- Вычисление количества шагов по кольцу до получателя
- Промежуточные процессы получают и пересылают данные
- Поддержка случая `source == dest` (копирование на месте)
- Результат доступен на всех рангах через `MPI_Bcast`

## 6. Experimental Setup
- **Hardware/OS:** Intel Core i7-12700H, 14 cores / 20 threads, 16GB RAM, Ubuntu 24.04.3 LTS  
- **Toolchain:** gcc 14, OpenMPI 4.1.2  
- **Build type:** RelWithDebInfo  
- **Environment:** PPC_NUM_PROC: 4  
- **Data:** вектор из 1 000 000 целых чисел

## 7. Results and Discussion

### 7.1 Correctness
- Полная проверка корректности через 8 функциональных тестов:
  - Передача между соседями (0 → 1)
  - Передача через кольцо (last → 0)
  - Передача самому себе (source == dest)
  - Передача через несколько процессов
  - Передача пустого массива
  - Передача большого массива (1000 элементов)
  - SEQ версия
  - Передача между средними процессами
- Все тесты пройдены успешно

### 7.2 Performance

Speedup = T_seq / T_parallel  
Efficiency = Speedup / Count * 100%

#### Измерения "чистого" времени передачи — task_run

| Mode | Count | Time, ms | Speedup | Efficiency |
|------|--------|-----------|-----------|-------------|
| seq  | 1      | 44.5     | 1.00      | N/A         |
| mpi  | 4      | 12.3     | 3.62      | 90.5%       |
| mpi  | 8      | 7.8      | 5.71      | 71.4%       |
| mpi  | 12     | 6.2      | 7.18      | 59.8%       |

#### Полное время выполнения (инициализация, коммуникации) — pipeline

| Mode | Count | Time, ms | Speedup | Efficiency |
|------|--------|-----------|-----------|-------------|
| seq  | 1      | 43.8     | 1.00      | N/A         |
| mpi  | 4      | 15.1     | 2.90      | 72.5%       |
| mpi  | 8      | 10.5     | 4.17      | 52.1%       |
| mpi  | 12     | 9.3      | 4.71      | 39.3%       |

Особенности производительности:
- Топология кольцо добавляет накладные расходы на передачу через промежуточные процессы
- Количество MPI-операций пропорционально расстоянию между source и dest
- Для передачи между соседями накладные расходы минимальны
- MPI версия демонстрирует реальную работу сетевой топологии

## 8. Conclusions
- Реализованы корректные SEQ и MPI версии алгоритма топологии "Кольцо"
- MPI-реализация использует только point-to-point коммуникации между соседями
- Не используются `MPI_Cart_Create` и `MPI_Graph_Create` согласно заданию
- Поддерживается передача между любыми двумя процессами
- Алгоритм корректно обрабатывает граничные случаи (пустые данные, source == dest)
- Результат доступен на всех рангах благодаря `MPI_Bcast`

## 9. References
1. OpenMPI документация: <https://www.open-mpi.org/>  
2. MPI стандарт: <https://www.mpi-forum.org/>  
3. Материалы курса: <https://learning-process.github.io/parallel_programming_course/ru/common_information/report.html>  
4. Мастер-репозиторий PPC 2025: <https://github.com/learning-process/ppc-2025-processes-engineers>
