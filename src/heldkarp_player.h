#ifndef HELDKARP_PLAYER_H
#define HELDKARP_PLAYER_H

#include "graph.h"
#include "move.h"

struct hexa_pos {
    int q;
    int r;
};

unsigned int **compute_cost_matrix(unsigned int start, unsigned int *objectives, unsigned int n_obj,
                                   enum dir_t init_dir);

unsigned int *held_karp_tsp(unsigned int **dist, unsigned int n);

int fill_path_opponent(unsigned int *best_path, unsigned int *size_path, unsigned int *position,
                       unsigned int initial_start, enum dir_t *last_dir);

void fill_path_player(unsigned int *best_path, unsigned int *size_path, unsigned int *position,
                      unsigned int initial_start, enum dir_t *last_dir);

unsigned int get_position_player();

void build_index_to_hex_table(struct hexa_pos *table, int m);

int hexagonal_distance(int i, int j, struct hexa_pos *index_to_hex);

void fill_heuristique(unsigned int *heuristique, unsigned int n, struct hexa_pos *index_to_hex);

int cmp_length(unsigned int d1, unsigned int d2);

unsigned int extractMin(unsigned int *couleur, unsigned int *d, unsigned int *h, int n);

void release(unsigned int x, unsigned int j, unsigned int length, unsigned int *d,
             unsigned int *parent, unsigned int *nonNoir);

unsigned int direction(vertex_t a, vertex_t b);

unsigned int max_lentgh_move(enum dir_t before, enum dir_t after);

void outgoing_edge_list(enum dir_t before, unsigned int v, vertex_t *tab, unsigned int *dist);

int is_in_dir(vertex_t deb, vertex_t fin, unsigned int dir, int l);

unsigned int *A_star(unsigned int start, unsigned int end, enum dir_t initial_dir);

int is_empty_color(unsigned int *couleur, unsigned int c, int n);

unsigned int get_close_neighbor(unsigned int start, unsigned int dest);

unsigned int get_edge_neighbors(struct edge_t *neighbors, struct edge_t e);

int min(int a, int b);

int max(int a, int b);

int dot_product(int ax, int ay, int bx, int by);

void cpy_A_star_result(unsigned int *orig, unsigned int *dest);

void follow_path(unsigned int *path, unsigned int *size_path, unsigned int *position,
                 enum dir_t *last_dir, unsigned int color);

int wall_impact_path(unsigned int *path, unsigned int *size_path, struct edge_t edge1,
                     struct edge_t edge2);

enum move_type_t action_type(struct edge_t *edge1, struct edge_t *edge2);

struct move_t action(enum move_type_t act_t, struct edge_t edge1, struct edge_t edge2);

#endif
