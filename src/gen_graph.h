#ifndef GRAPH_H
#define GRAPH_H

#include "graph.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Fonctions pour manipuler le graphe
struct graph_t *graph_create(unsigned int num_vertices);
void graph_destroy(struct graph_t *graph);
bool graph_add_edge(struct graph_t *graph, struct edge_t edge, enum dir_t dir);
bool graph_add_edge_compressed(struct graph_t *graph, struct edge_t edge, enum dir_t dir);
bool graph_remove_edge(struct graph_t *graph, struct edge_t edge);
void manual_edge_set(struct graph_t *graph, struct edge_t edge, enum dir_t replace);
enum dir_t graph_get_edge_direction(struct graph_t *graph, struct edge_t edge);
bool graph_has_edge(struct graph_t *graph, struct edge_t edge);
bool graph_add_objectives(struct graph_t *graph, unsigned int max_objectives, unsigned int *tab);

// Fonction pour s'assurer que la matrice est au format CSR
void graph_ensure_csr_format(struct graph_t *graph);

// Fonctions pour créer différents types de graphes
struct graph_t *graph_create_triangular(unsigned int m);
struct graph_t *graph_create_cyclic(unsigned int m);
// struct graph_t *graph_create_holed(unsigned int m);

// Fonctions pour parcourir le graphe
unsigned int *graph_get_neighbors(struct graph_t *graph, unsigned int vertex, unsigned int *count);
bool graph_has_path(struct graph_t *graph, unsigned int from, unsigned int to);

// Fonction pour afficher la matrice d'adjacence complète
void print_adjacency_matrix(struct graph_t *graph);

// Fonction pour afficher les informations du graphe
void print_graph_info(struct graph_t *graph);

#endif // GRAPH_H
