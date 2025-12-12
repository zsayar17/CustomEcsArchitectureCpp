# Custom ECS (Entity Component System) Framework

A high-performance Entity Component System (ECS) implementation built from scratch in C++. This project strictly follows **Data-Oriented Programming (DOP)** principles to align data in memory, specifically optimizing for CPU cache efficiency (L1/L2/L3 cache).

## 🚀 Key Features

* **Custom Memory Management:** Uses 16KB Chunks with aligned memory addressing.
* **SoA Architecture:** Data is stored in a Structure of Arrays (SoA) layout within chunks for faster iteration.
* **Dynamic Composition:** Create/destroy entities and add/remove components dynamically at runtime (safe for use within loops).
* **Multithreading Support:** Job system implementation that leverages the aligned chunk architecture for thread-safe parallel processing.
* **Automatic Defragmentation:** Intelligent handling of structural changes to keep memory blocks full and efficient.

---

## 🏗️ Architecture Overview

The system relies on three core structures: **Component**, **Chunk**, and **Archetype**.

### 1. Components & Chunks
* **Components:** Pure data structures containing entity data.
* **Chunks:** The fundamental unit of memory storage.
    * **Size:** Fixed at **16KB** to fit efficiently into CPU cache lines.
    * **Alignment:** Uses 16-byte aligned addressing.
    * **Layout:** Stores components in a **Structure of Arrays (SoA)** format.
    * **Header:** Contains metadata such as capacity, the owner Archetype, and the current entity count.

### 2. Archetypes
Archetypes act as containers for entities that share the same unique signature (a specific combination of component types).
* **Chunk Management:** Archetypes manage a list of Chunks. When a chunk is filled, the Archetype dynamically allocates a new one.
* **Structural Changes:** When components are added/removed, the entity moves between Archetypes.
* **Memory Efficiency:** If an entity is destroyed, the system performs a swap-and-pop operation or realigns the data to fill "holes" in the memory block. This ensures chunks remain as dense as possible to maintain high cache hit efficiency.

### 3. EntityManager
The main interface for the developer. It abstracts the complexity of the underlying memory management.
* Manages Archetype lifecycles.
* Handles entity creation and destruction.
* Provides utilities to query entity details (Archetype, Chunk location, Index).

---

## ⚙️ Systems & Logic

### Query Interface
A robust structure allowing developers to retrieve specific data subsets. Queries filter entities based on their component composition, returning only the relevant data arrays from the Archetypes.

### Systems
Logic containers that iterate over data. Systems registered to the `EntityManager` are executed in the main game loop. They operate on the data retrieved via the Query interface.

### Job System (Multithreading)
The framework supports executing logic on multiple threads ("Jobs").
* Thanks to the **aligned chunk architecture**, data races are minimized.
* The system schedules jobs without compromising the cache hit rate, allowing for massive scalability in complex simulations.

--

![Architecure](image.png)


![BenchMark (4.000.000 Entities -> 100 iterations)](image-1.png)

![BenchMark (4.000.000 Entities -> 1000 iterations](image-2.png)
