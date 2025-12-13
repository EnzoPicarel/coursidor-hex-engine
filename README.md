# Coursidor

Coursidor is a variant of the Quoridor board game.  
Two players (BLACK/0 and WHITE/1) traverse a board defined by a graph with triangular or cyclic tiling, must validate a set of objectives (bases) in the order of their choice, then return to their starting point. Each turn, a player can move their piece along a valid edge or place a wall to slow down the opponent without completely blocking them.

---

## Project Information

- Subject page:  
    <https://www.labri.fr/perso/renault/working/teaching/projets/2024-25-S6-C-Coursidor.php>
- Project page on Thor:  
    <https://thor.enseirb-matmeca.fr/ruby/projects/pr105-coursidor>

## Compilation

Compile the server and player libraries:
```bash
make build GSL_PATH=$GSL_PATH
```

Compile unit tests:
```bash
make build_tests GSL_PATH=$GSL_PATH
```

## Installation

```bash
make install
```

After make install, the install/ directory will contain:

* server – the server executable
* alltests – all tests executable
* *.so – client libraries

## Execution

For quick execution:
```bash
make exec
```

### Otherwise:

#### Syntax:

```bash
./install/server [-m width] [-t type] [-M max_rounds] [-O num_objectives] <player1.so> <player2.so>
```

#### Example:

Triangular graph of size 5, 50 rounds and 3 objectives

```bash
./install/server -m 5 -t T -M 50 -O 3 install/astar_player.so install/random_player.so
```

## Tests

```bash
make test
```

## Game Rules

* Objective: each player (BLACK=0, WHITE=1) must visit all bases (objectives) then return to their starting square.
* Movement (MOVE):
    * by default, move to an adjacent vertex.
    * if same direction as previous move: up to 3 vertices.
    * if at 30°: up to 2 vertices.
    * jump over opponent possible if adjacent.
* Wall placement (WALL):
    * two consecutive edges around a vertex.
    * removes these edges from the graph.
    * cannot completely block a player (must always keep a path to reach objectives and base).
* End conditions:
    * a player makes an invalid move → loses immediately.
    * a player validates all objectives and returns to their base → wins.
    * maximum number of rounds reached → draw.

## Authors

Enzo Picarel, Raphaël Bely, Arno Donias, Thibault Abeille