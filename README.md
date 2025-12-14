<div align="center">
  <h3 align="center">Coursidor Hex Engine</h3>

  <p align="center">
    A strategic C-based board game engine featuring <strong>sparse graph processing</strong>, <strong>dynamic library loading</strong>, and <strong>A* pathfinding</strong>.
    <br />
    <a href="#-getting-started"><strong>Get Started »</strong></a>
  </p>
  
  ![CI Status](https://img.shields.io/badge/build-passing-brightgreen)
  ![License](https://img.shields.io/badge/license-MIT-blue)
</div>

## 🔍 About The Project
Coursidor is a variation of the Quoridor board game played on a graph with triangular or cyclic tiling. 

Two players (BLACK/0 and WHITE/1) must validate a set of objectives and return to their base. The core challenge lies in the **Graph Theory** constraints: placing walls removes edges from the adjacency matrix without disconnecting the graph (Reachability Check).

*Built as a Semester 6 project at ENSEIRB-MATMECA.*

### 🛠 Built With
* **Language:** C (C99)
* **Build System:** GNU Make
* **Libraries:** GSL (GNU Scientific Library - Sparse Matrices), `libm`
* **System:** POSIX Dynamic Loading (`dlopen`/`dlsym`)

## 📐 Architecture & Technical Highlights
* **Dynamic Strategy Loading:** Player AIs are compiled as separate shared libraries (`.so`) and loaded by the server at runtime using `dlopen`. This allows for hot-swapping strategies without recompiling the engine.
* **Sparse Matrix Graph:** The board is represented as a sparse graph (CSR format) to optimize memory for large grid sizes.
* **Pathfinding:** Implements **A\*** (A-Star) with a hexagonal distance heuristic and **Held-Karp** (TSP) for optimal objective ordering.

## 🚀 Getting Started

### Prerequisites
* `gcc`
* `make`
* `libgsl-dev` (GNU Scientific Library)

### Installation & Compilation

1. **Build the Engine & Tests**
   ```bash
   # Compiles server, player libraries (.so), and unit tests
   make build GSL_PATH=$GSL_PATH
   make build_tests GSL_PATH=$GSL_PATH
   ```

2. **Install Artifacts**
   ```bash
   # Moves executables and libs to ./install directory
   make install
   ```

## ⚡ Execution

**Quick Run (Default settings):**
```bash
make exec
```

**Advanced Run:**
Run the server with custom graph size (`-m`), max rounds (`-M`), or specific player strategies (`.so` files).

```bash
# Syntax
./install/server [-m width] [-t type] [-M max_rounds] [-O num_objectives] <player1.so> <player2.so>

# Example: Size 5, 50 rounds, 3 objectives, A* vs Random
./install/server -m 5 -t T -M 50 -O 3 install/astar_player.so install/random_player.so
```

## 🧪 Tests
Run the unit test suite (Graph validation, Wall logic, Memory leaks):
```bash
make test
```

## 📜 Game Rules
* **Objective:** Visit all bases (objectives) and return to start.
* **Movement:**
    * Standard: Adjacent vertex.
    * Momentum: Up to 3 vertices if maintaining direction.
    * 30° turn: Up to 2 vertices.
    * Jump over opponent if adjacent.
* **Wall Placement:**
    * Removes 2 edges around a vertex.
    * **Constraint:** Cannot completely block a player's path to any objective (Graph connectivity check).
* **End Conditions:** Invalid move (Loss), Objectives cleared + Return (Win), Max rounds (Draw).

## 👥 Authors

* **Enzo Picarel**
* **Raphaël Bely**
* **Arno Donias**
* **Thibault Abeille**

---
*Original Project Specs: [Labri Subject Page](https://www.labri.fr/perso/renault/working/teaching/projets/2024-25-S6-C-Coursidor.php)*