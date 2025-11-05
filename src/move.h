#ifndef _CORS_MOVE_H_
#define _CORS_MOVE_H_

#include <stddef.h>
#include <stdint.h>

/* The number of players in the game */
#define NUM_PLAYERS 2

/* The possible player colors */
enum player_color_t { BLACK = 0, WHITE = 1, NO_COLOR = 2 };

/* The different types of moves */
enum move_type_t { NO_TYPE = 0, WALL = 1, MOVE = 2 };

/* The ids of the vertices in the graph, ranging between 0 and the
   number of vertices - 1 */
typedef unsigned int vertex_t;

/* The representation of an edge in the graph */
struct edge_t {
    vertex_t fr, to;
};

/* A move in the game */
struct move_t {
    enum player_color_t c; // The color of the player
    enum move_type_t t;    // The type of the move
    vertex_t m;            // The id of a vertex
    struct edge_t e[2];    // The two edges for the wall
};

#endif // _CORS_MOVE_H_
