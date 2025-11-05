# Coursidor

Coursidor est une variante du jeu de Quoridor.  
Deux joueurs (BLACK/0 et WHITE/1) parcourent un plateau défini par un graphe à pavage triangulaire ou cyclique, doivent valider un ensemble d’objectifs (bases) dans l’ordre de leur choix, puis revenir à leur point de départ. À chaque tour, un joueur peut déplacer son pion le long d’une arête valide ou poser un mur pour ralentir l’adversaire sans le bloquer complètement.

---

## Sujet du projet

- Page du sujet :  
    <https://www.labri.fr/perso/renault/working/teaching/projets/2024-25-S6-C-Coursidor.php>
- Page du projet sur Thor :  
    <https://thor.enseirb-matmeca.fr/ruby/projects/pr105-coursidor>

## Compilation

Compile le serveur et les bibliothèques joueurs :
```bash
make build GSL_PATH=$GSL_PATH
```

Compile les tests unitaires :
```bash
make build_tests GSL_PATH=$GSL_PATH
```

## Installation 

```bash
make install
```

Après make install, le répertoire install/ contiendra :

* server – l’exécutable du serveur
* alltests – l’exécutable de tous les tests
* *.so – les bibliothèques clientes

## Exécution

Pour une exécution rapide :
```bash
make exec
```

### Sinon :

#### Syntaxe :

```bash
./install/server [-m width] [-t type] [-M max round] [-O num objectives] <player1.so> <player2.so>
```

#### Exemple :

Graphe triangulaire de taille 5, 50 tours et 3 objectifs

```bash
./install/server -m 5 -t T -M 50 -o 3 install/player1.so install/player2.so
```

## Tests

```bash
make test
```

## Règles du jeu

* Objectif : chaque joueur (BLACK=0, WHITE=1) doit visiter toutes les bases (objectifs) puis revenir à sa case de départ.
* Déplacement (MOVE) :
    * par défaut, déplacement sur un sommet adjacent.
    * si même direction que le coup précédent : jusqu’à 3 cases.
    * si à 30° : jusqu’à 2 cases.
    * saut par-dessus l’adversaire possible si adjacent.
* Placement de mur (WALL) :
    * deux arêtes consécutives autour d’un sommet.
    * retire ces arêtes du graphe.
    * ne peut pas bloquer totalement un joueur (doit toujours garder un chemin pour atteindre objectifs et base).
* Conditions de fin :
    * un joueur joue un coup invalide → il perd immédiatement.
    * un joueur valide tous les objectifs et revient à sa base → il gagne.
    * nombre maximal de tours atteint → match nul.

## Auteurs

Enzo Picarel, Raphaël Bely, Arno Donias, Thibault Abeille