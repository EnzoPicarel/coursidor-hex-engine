#include "random_player.h"
#include "gen_graph.h"
#include "player.h"
#include <stdlib.h>

#define UNUSED(x) (void)(x)

#ifndef INF
#define INF 10000000
#endif // INF

static struct graph_t *graph_player;
static unsigned int origin_position_player;
static unsigned int origin_position_opponent;
static unsigned int position_player;
static unsigned int position_opponent;
static unsigned int color_player;
static unsigned int color_opponent;
static unsigned int *visited_objective_player;
static unsigned int *visited_objective_opponent;
// static int current_round;

char const *get_player_name() {
    return "RANDOM JEAN";
}

void initialize(unsigned int player_id, struct graph_t *graph) {
    graph_player = graph;

    visited_objective_player = calloc(graph->num_objectives, sizeof(unsigned int));
    visited_objective_opponent = calloc(graph->num_objectives, sizeof(unsigned int));

    color_player = player_id;
    if (color_player == BLACK)
        color_opponent = WHITE;
    else
        color_opponent = BLACK;
    position_player = graph_player->start[color_player];
    origin_position_player = graph_player->start[color_player];
    position_opponent = graph_player->start[color_opponent];
    origin_position_opponent = graph_player->start[color_opponent];
    // current_round = 1;
}

struct move_t play(const struct move_t previous_move) {
    position_opponent = previous_move.m;
    if (previous_move.t == 1) {
        // place the wall in the graph player
        graph_remove_edge(graph_player, previous_move.e[0]);
        graph_remove_edge(graph_player, previous_move.e[1]);
    } else if (previous_move.t == 2) {
        // change the position_opponent
        position_opponent = previous_move.m;
        for (unsigned int i = 0; i < graph_player->num_objectives; i++) {
            if (position_opponent == graph_player->objectives[i])
                visited_objective_opponent[i] = 1;
        }
    }

    // gather neighbors with the get neighbors function
    unsigned int nb_neighbors;
    unsigned int *neighbors = graph_get_neighbors(graph_player, position_player, &nb_neighbors);

    // chose a random neighbor as next move
    unsigned int neighbor = position_player; // default to current position
    if (nb_neighbors > 0) {
        do {
            unsigned int random_index = rand() % nb_neighbors;
            neighbor = neighbors[random_index];
        } while (neighbor == position_opponent);
    }

    // free the neighbors array
    free(neighbors);

    // update the position_player
    position_player = neighbor;
    for (unsigned int i = 0; i < graph_player->num_objectives; i++) {
        if (position_player == graph_player->objectives[i])
            visited_objective_player[i] = 1;
    }

    // create the move
    struct edge_t w1 = {0, 0};
    struct edge_t w2 = {0, 0};
    /*
    if (current_round == 1)
    {
        w1.fr = 0;
        w1.to = 4;
        w2.fr = 0;
        w2.to = 5;
    }
    else {
        w1.fr = 0;
        w1.to = 1;
        w2.fr = 0;
        w2.to = 5;
    }
    current_round++;*/
    struct move_t next_move = {.c = color_player, .t = 2, .m = position_player, .e = {w1, w2}};
    // struct move_t next_move = {.c = color_player, .t = 1, .m = position_player, .e = {w1, w2}};

    return next_move;
}

void finalize() {
    graph_destroy(graph_player);
    free(visited_objective_player); // Free the objective tracking arrays
    free(visited_objective_opponent);
}

unsigned int get_position_player() {
    return position_player;
}