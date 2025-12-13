#include "astar_player.h"
#include "gen_graph.h"
#include "player.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define UNUSED(x) (void)(x)

#ifndef INF
#define INF 10000000
#endif // INF

enum visit_state_t
{
    EMPTY = 0,
    STATE_GRAY = 1,
    STATE_BLACK = 2,
};

static struct graph_t *graph_player;
static unsigned int origin_position_player;
static unsigned int origin_position_opponent;

static unsigned int color_player;
static unsigned int color_opponent;

static unsigned int *visited_objective_player;
static unsigned int *visited_objective_opponent;

static vertex_t *position_player;
static vertex_t *position_opponent;

static vertex_t *path_player;
static vertex_t *path_opponent;

static unsigned int *size_path_player;
static unsigned int *size_path_opponent;

static enum dir_t *previous_dir_player;
static enum dir_t *previous_dir_opponent;

static unsigned int nb_wall_placed;

// PLAYING

char const *get_player_name()
{
    return "JEAN ASTAR";
}

void initialize(unsigned int player_id, struct graph_t *graph)
{
    srand((unsigned)time(NULL));
    graph_player = graph;

    visited_objective_player = calloc(graph->num_objectives, sizeof(unsigned int));
    visited_objective_opponent = calloc(graph->num_objectives, sizeof(unsigned int));

    color_player = player_id;
    if (color_player == BLACK)
        color_opponent = WHITE;
    else
        color_opponent = BLACK;
    position_player = malloc(sizeof(vertex_t));
    *position_player = graph_player->start[color_player];
    path_player = calloc(graph_player->num_vertices, sizeof(unsigned int));
    for (unsigned int i = 0; i < graph_player->num_vertices; i++)
    {
        path_player[i] = 666;
    }
    origin_position_player = graph_player->start[color_player];
    previous_dir_player = malloc(sizeof(enum dir_t));
    *previous_dir_player = NO_EDGE;
    size_path_player = malloc(sizeof(unsigned int));
    *size_path_player = 0;

    position_opponent = malloc(sizeof(vertex_t));
    *position_opponent = graph_player->start[color_opponent];
    path_opponent = calloc(graph_player->num_vertices, sizeof(unsigned int));
    origin_position_opponent = graph_player->start[color_opponent];
    previous_dir_opponent = malloc(sizeof(enum dir_t));
    *previous_dir_opponent = NO_EDGE;
    size_path_opponent = malloc(sizeof(unsigned int));
    *size_path_opponent = 0;

    nb_wall_placed = 0;
}

unsigned int get_position_player()
{
    return *position_player;
}

struct move_t play(const struct move_t previous_move)
{
    struct move_t next_move;
    if (previous_move.t == NO_TYPE || previous_move.c == NO_COLOR)
    {
        fill_path(path_player, size_path_player, visited_objective_player, position_player,
                  origin_position_player, previous_dir_player);
        fill_path(path_opponent, size_path_opponent, visited_objective_opponent, position_opponent,
                  origin_position_opponent, previous_dir_opponent);
        follow_path(path_player, size_path_player, position_player, previous_dir_player,
                    visited_objective_player);
        next_move.c = color_player;
        next_move.t = 2;
        next_move.m = *position_player;
        next_move.e[0].fr = 0;
        next_move.e[0].to = 0;
        next_move.e[1].fr = 0;
        next_move.e[1].to = 0;
    }
    else if (previous_move.t == WALL)
    {
        graph_remove_edge(graph_player, previous_move.e[0]);
        graph_remove_edge(graph_player, previous_move.e[1]);
        if (wall_impact_path(path_player, size_path_player, previous_move.e[0],
                             previous_move.e[1]))
        {
            fill_path(path_player, size_path_player, visited_objective_player, position_player,
                      origin_position_player, previous_dir_player); // refresh path after wall
        }
        struct edge_t edge1;
        struct edge_t edge2;
        enum move_type_t act_t = action_type(&edge1, &edge2);
        next_move = action(act_t, edge1, edge2);
    }
    else if (previous_move.t == MOVE)
    {
        vertex_t old_position_opponent = *position_opponent;
        *position_opponent = previous_move.m;
        *previous_dir_opponent = get_direction(old_position_opponent, *position_opponent);
        if (*size_path_opponent <= 1)
        {
            fill_path(path_opponent, size_path_opponent, visited_objective_opponent,
                      position_opponent, origin_position_opponent, previous_dir_opponent);
        }
        if (*position_opponent == path_opponent[(*size_path_opponent) - 1])
        {
            (*size_path_opponent)--;
        }
        else
        {
            fill_path(path_opponent, size_path_opponent, visited_objective_opponent,
                      position_opponent, origin_position_opponent, previous_dir_opponent);
        }

        for (unsigned int i = 0; i < graph_player->num_objectives; i++)
        {
            if (*position_opponent == graph_player->objectives[i])
            {
                visited_objective_opponent[i] = 1;
            }
        }
        struct edge_t edge1;
        struct edge_t edge2;
        enum move_type_t act_t = action_type(&edge1, &edge2);
        next_move = action(act_t, edge1, edge2);
    }
    return next_move;
}

void finalize()
{
    graph_destroy(graph_player);
    free(position_player);
    free(position_opponent);
    free(path_player);
    free(path_opponent);
    free(previous_dir_opponent);
    free(previous_dir_player);
    free(size_path_player);
    free(size_path_opponent);
    free(visited_objective_player);
    free(visited_objective_opponent);
}

// A_STAR
unsigned int *a_star(unsigned int start, unsigned int end, enum dir_t initial_dir)
{
    // Returns an array shaped like: [path length, end, ..., start]
    unsigned int n = graph_player->num_vertices;

    unsigned int *d = calloc(n, sizeof(unsigned int)); // best known distance
    for (unsigned int i = 0; i < n; i++)
    {
        d[i] = INF;
    }
    d[start] = 0;

    vertex_t *parent = calloc(n, sizeof(vertex_t));
    for (unsigned int i = 0; i < n; i++)
    {
        parent[i] = n; // n represents "no parent"
    }

    enum dir_t *previous_dir = calloc(n, sizeof(enum dir_t));
    for (unsigned int i = 0; i < n; i++)
    {
        previous_dir[i] = NO_EDGE;
    }
    previous_dir[start] = initial_dir;

    unsigned int *h = malloc(sizeof(unsigned int) * n);
    struct hex_pos *h_pos = calloc(n, sizeof(struct hex_pos));
    build_index_to_hex_table(h_pos, (int)((3 + sqrt(12 * n - 3)) / 6));
    fill_heuristic(h, end, h_pos);
    free(h_pos);

    vertex_t *visit_state = calloc(n, sizeof(vertex_t));
    visit_state[start] = STATE_GRAY;

    while (1)
    {
        if (is_empty_color(visit_state, STATE_GRAY, n))
            break;

        unsigned int x = extract_min(visit_state, d, h, n);
        if (x == end)
            break;

        visit_state[x] = STATE_BLACK;

        unsigned int *vertices = calloc(10, sizeof(unsigned int));
        unsigned int *dist = calloc(10, sizeof(unsigned int));

        outgoing_edge_list(previous_dir[x], x, vertices, dist);

        for (unsigned int j = 0; j < 10 && vertices[j] != INF; j++)
        {
            unsigned int y = vertices[j];
            // unsigned int weight = dist[j];

            if (d[x] + 1 < d[y])
            {
                d[y] = d[x] + 1;
                parent[y] = x;
                previous_dir[y] = get_direction(x, y); // Update direction dynamically
                visit_state[y] = STATE_GRAY;
            }
        }

        free(vertices);
        free(dist);
    }

    if (parent[end] == n && end != start)
    {
        unsigned int *fail = calloc(2, sizeof(unsigned int));
        fail[0] = 1;
        fail[1] = start;
        free(d);
        free(parent);
        free(previous_dir);
        free(visit_state);
        free(h);
        return fail;
    }

    vertex_t *way = calloc(n + 1, sizeof(vertex_t));
    way[0] = 0;
    way[1] = end;
    while (end != start && way[0] < n - 1)
    {
        way[0]++;
        way[way[0]] = end;
        end = parent[end];
    }
    way[0]++;
    way[way[0]] = start;

    free(d);
    free(parent);
    free(previous_dir);
    free(visit_state);
    free(h);

    return way;
}

// OTHER FUNCTION

void outgoing_edge_list(enum dir_t before, unsigned int v, vertex_t *tab, unsigned int *dist)
{
    static const enum dir_t DirOrder[6] = {NW, NE, W, E, SE, SW};
    int n = 0;

    for (int d = 0; d < 6; d++)
    {
        enum dir_t dir = DirOrder[d];

        // find the immediate neighbor in direction 'dir'
        vertex_t curr = INF;
        for (int k = graph_player->t->p[v]; k < graph_player->t->p[v + 1]; k++)
        {
            if ((enum dir_t)graph_player->t->data[k] == dir)
            {
                curr = graph_player->t->i[k];

                if (curr != *position_opponent)
                {
                    tab[n] = curr;
                    dist[n] = 1;
                    n++;
                }

                unsigned int maxd = max_length_move(before, dir);
                vertex_t prev = curr;

                // chain steps 2 and 3
                for (unsigned int step = 2; step <= maxd; step++)
                {
                    vertex_t next = INF;
                    for (int j = graph_player->t->p[prev]; j < graph_player->t->p[prev + 1]; j++)
                    {
                        if ((enum dir_t)graph_player->t->data[j] == dir)
                        {
                            next = graph_player->t->i[j];
                            break;
                        }
                    }
                    if (next == INF)
                        break; // no further edge
                    if (next != *position_opponent)
                    {
                        tab[n] = next;
                        dist[n] = step;
                        n++;
                    }
                    prev = next;
                }
                break;
            }
        }
    }
    // sentinel
    if (n < 10)
        tab[n] = INF;
}

unsigned int extract_min(unsigned int *visit_state, unsigned int *d, unsigned int *h, int n)
{
    // Returns the index of the smallest distance still marked as gray (open set)
    unsigned int min = 0;
    while (visit_state[min] != STATE_GRAY)
        min += 1;
    for (unsigned int i = min; (int)i < n; i++)
    {
        if (visit_state[i] == STATE_GRAY && cmp_length(d[i] + h[i], d[min] + h[min]))
            min = i;
    }
    return min;
}

void relax(unsigned int x, unsigned int y, unsigned int length, unsigned int *d,
           unsigned int *parent, unsigned int *visit_state)
{
    if (cmp_length(length + d[x], d[y]))
    {
        d[y] = length;
        parent[y] = x;
        visit_state[y] = STATE_GRAY;
    }
}

void build_index_to_hex_table(struct hex_pos *table, int m)
{
    unsigned int idx = 0;

    // Walk line by line along the q axis (horizontal)
    for (int r = -m + 1; r <= m - 1; r++)
    {
        int q_min = maximum(-m + 1, -r - m + 1);
        int q_max = minimum(m - 1, -r + m - 1);

        for (int q = q_min; q <= q_max; q++)
        {
            if (idx >= graph_player->num_vertices)
            {
                return;
            }
            table[idx].q = q;
            table[idx].r = r;
            idx++;
        }
    }
}

// Helper usage:
// struct hex_pos *index_to_hex = malloc(num_vertices * sizeof(struct hex_pos));
// build_index_to_hex_table(table, m);
// hex_pos pos_i = index_to_hex[i] retrieves the coordinates for vertex i

int hexagonal_distance(int i, int j, struct hex_pos *index_to_hex)
{
    int qi = index_to_hex[i].q;
    int ri = index_to_hex[i].r;
    int qj = index_to_hex[j].q;
    int rj = index_to_hex[j].r;
    if (abs(qi - qj) >= abs(ri - rj) && abs(qi - qj) >= abs((qi + ri) - (qj + rj)))
    {
        return abs(qi - qj);
    }
    else if (abs(ri - rj) >= abs(qi - qj) && abs(ri - rj) >= abs((qi + ri) - (qj + rj)))
    {
        return abs(ri - rj);
    }
    else
    {
        return abs((qi + ri) - (qj + rj));
    }
}

void fill_heuristic(unsigned int *heuristic, unsigned int n, struct hex_pos *index_to_hex)
{
    int m = graph_player->num_vertices;
    for (int i = 0; i < m; i++)
    {
        heuristic[i] = (unsigned int)floor(hexagonal_distance(i, n, index_to_hex) / 3);
    }
}

unsigned int get_close_neighbor(unsigned int start, unsigned int dest)
{
    struct hex_pos *index_to_hex =
        malloc(graph_player->num_vertices * sizeof(struct hex_pos)); // TO BE FREE 11
    build_index_to_hex_table(index_to_hex,
                             (int)((3 + sqrt(12 * graph_player->num_vertices - 3)) / 6));

    unsigned int count;
    unsigned int *neighbors = graph_get_neighbors(graph_player, start, &count); // TO BE FREE 12
    if (neighbors == NULL || count == 0)
    {
        free(index_to_hex);
        return start;
    }
    struct hex_pos start_pos = index_to_hex[start];
    struct hex_pos dest_pos = index_to_hex[dest];
    // Direction vector
    int vq = dest_pos.q - start_pos.q;
    int vr = dest_pos.r - start_pos.r;

    double best_dot = -1e9;
    unsigned int close_neighbor = neighbors[0]; // default pick

    for (unsigned int i = 0; i < count; i++)
    {
        unsigned int neighbor = neighbors[i];
        struct hex_pos neigh_pos = index_to_hex[neighbor];
        int nvq = neigh_pos.q - start_pos.q;
        int nvr = neigh_pos.r - start_pos.r;
        // Compute dot product
        int dot = dot_product(vq, vr, nvq, nvr);

        if (dot > best_dot)
        {
            best_dot = dot;
            close_neighbor = neighbor;
        }
    }
    free(neighbors);    // FREE 12
    free(index_to_hex); // FREE 11
    return close_neighbor;
}

unsigned int get_edge_neighbors(struct edge_t *neighbors, struct edge_t e)
{
    unsigned int nb_n_v1 = 0;
    unsigned int nb_n_v2 = 0;
    unsigned int *n_v1 = graph_get_neighbors(graph_player, e.fr, &nb_n_v1); // TO BE FREE 13
    unsigned int *n_v2 = graph_get_neighbors(graph_player, e.to, &nb_n_v2); // TO BE FREE 14
    unsigned int count = 0;
    for (unsigned int i = 0; i < nb_n_v1; i++)
    {
        for (unsigned int j = 0; j < nb_n_v2; j++)
        {
            if (n_v1[i] == n_v2[j])
            {
                if (get_direction(e.fr, n_v1[i]) != 0)
                {
                    struct edge_t e1 = {e.fr, n_v1[i]};
                    neighbors[count] = e1;
                    count += 1;
                }
                if (get_direction(n_v1[i], e.to) != 0)
                {
                    struct edge_t e2 = {e.to, n_v1[i]};
                    neighbors[count] = e2;
                    count += 1;
                }
            }
        }
    }
    free(n_v1); // FREE 13
    free(n_v2); // FREE 14
    return count;
}

int minimum(int a, int b)
{
    return a < b ? a : b;
}
int maximum(int a, int b)
{
    return a > b ? a : b;
}

int dot_product(int ax, int ay, int bx, int by)
{
    return ax * bx + ay * by;
}

int cmp_length(unsigned int d1, unsigned int d2)
{
    // Returns 1 if d1 < d2, 0 otherwise (0 corresponds to infinity here)
    if (d1 >= INF)
        return 0;
    else if (d2 >= INF)
        return 1;
    else
        return (d1 < d2);
}

int is_in_direction(vertex_t deb, vertex_t fin, unsigned int dir, int l)
{
    if (l == 0)
        return 0;
    for (int k = graph_player->t->p[(int)deb]; k < graph_player->t->p[(int)deb + 1]; k++)
    {
        vertex_t next = graph_player->t->i[k];
        enum dir_t d = gsl_spmatrix_uint_get(graph_player->t, deb, next);
        if (d == dir)
        {
            if (next == fin)
                return 1;
            else if (d == NO_EDGE)
                return 0;
            else if (is_in_direction(next, fin, dir, l - 1))
                return 1;
        }
        if (d == NO_EDGE)
            return 0;
    }
    return 0;
}

unsigned int get_direction(vertex_t a, vertex_t b)
{
    // Return the direction to go from a to b
    for (unsigned int dir = FIRST_DIR; dir < LAST_DIR + 1; dir++)
    {
        if (is_in_direction(a, b, dir, 3))
            return dir;
    }
    return 0;
}

unsigned int max_length_move(enum dir_t before, enum dir_t after)
{
    // assert(before != 0 && after != 0);
    if (before == NO_EDGE || before == WALL_DIR)
        return 1;
    else if (before == after)
    {
        return 3;
    }
    else if (abs(((int)after - (int)before) % 6) == 1 ||
             abs(((int)after - (int)before) % 6) == 5)
    {
        return 2;
    }
    return 1;
}

int is_empty_color(unsigned int *visit_state, unsigned int c, int n)
{
    for (int i = 0; i < n; i++)
    {
        if (visit_state[i] == c)
            return 0;
    }
    return 1;
}

void copy_a_star_result(unsigned int *orig, unsigned int *dest)
{

    for (unsigned int j = 0; j <= orig[0]; j++)
    {
        dest[j] = orig[j];
    }
}

int fill_path(unsigned int *best_path, unsigned int *size_path, unsigned int *visited_objective,
              unsigned int *position, unsigned int initial_start, enum dir_t *last_dir)
{
    unsigned int n = graph_player->num_vertices;
    int back_to_base = 1;
    for (unsigned int j = 0; j < n; j++)
    {
        best_path[j] = INF;
    }
    best_path[0] = n;
    unsigned int one_objective_is_blocked = 0;

    for (unsigned int i = 0; i < graph_player->num_objectives; i++)
    {
        if (visited_objective[i] == 0)
        {
            back_to_base = 0;
            unsigned int *path_proposition =
                a_star(*position, graph_player->objectives[i], *last_dir);
            if (path_proposition[0] > 1 &&
                path_proposition[0] < best_path[0])
            { // consider this path if it is valid
                copy_a_star_result(path_proposition, best_path);
                best_path[0] = path_proposition[0];
            }
            else if (*position_opponent == initial_start)
            {
                if (*position_player != visited_objective[i] &&
                    path_proposition[0] <=
                        1)
                { // path invalid and not blocked by the opponent -> blocked by a wall
                    one_objective_is_blocked = 1;
                }
            }
            else if (*position_player == initial_start)
            {
                if (*position_opponent != visited_objective[i] &&
                    path_proposition[0] <=
                        1)
                { // path invalid and not blocked by the opponent -> blocked by a wall
                    one_objective_is_blocked = 1;
                }
            }
            // If the path is invalid only because the opponent is on it, it is still reachable
            free(path_proposition);
        }
    }

    unsigned int *path_proposition = a_star(*position, initial_start, *last_dir);
    if (*position_player == initial_start)
    {
        if (path_proposition[0] <= 1 && *position != initial_start &&
            *position_opponent !=
                initial_start)
        { // check if we can return to our starting point
            one_objective_is_blocked = 1;
        }
    }
    else
    {
        if (path_proposition[0] <= 1 && *position != initial_start &&
            *position_player != initial_start)
        { // check if we can return to the starting point
            one_objective_is_blocked = 1;
        }
    }

    if (back_to_base)
    {
        if (path_proposition[0] <= 1)
        {
            one_objective_is_blocked = 1;
        }
        copy_a_star_result(path_proposition, best_path);
    }
    free(path_proposition);
    *size_path = best_path[0];
    return one_objective_is_blocked;
}

void follow_path(unsigned int *path, unsigned int *size_path, unsigned int *position,
                 enum dir_t *last_dir, unsigned int *visited_objective)
{
    vertex_t old_position_player = *position;
    int blocked = 0;
    if (*size_path <= 1)
    {
        blocked = fill_path(path, size_path, visited_objective, position, origin_position_player,
                            last_dir);
        if (path[0] <= 1)
        {
            blocked = 1;
        }
    }
    if (path[0] == graph_player->num_vertices)
    {
        blocked = 1;
    }
    if (blocked)
    {
        unsigned int c;
        unsigned int *neighbor = graph_get_neighbors(graph_player, *position, &c);
        if (c != 0)
        {
            unsigned int idx = rand() % c;
            *position = neighbor[idx];
            *size_path = 0;
            *last_dir = get_direction(old_position_player, *position);
            if (*position_player == *position_opponent)
            {
                unsigned int c0;
                unsigned int *neighbor0 = graph_get_neighbors(graph_player, *position, &c0);
                for (unsigned int i = 0; i < c0; i++)
                {
                    if (is_in_direction(*position, neighbor0[i], *last_dir, 1))
                    {
                        *position = neighbor0[i];
                        break;
                    }
                }
                if (*position_player == *position_opponent)
                {
                    if (neighbor[0] != *position_opponent)
                    {
                        *position = neighbor[0];
                    }
                    else
                    {
                        *position = neighbor[1];
                    }
                }
                free(neighbor0);
            }
        }

        free(neighbor);
    }
    else
    {
        unsigned int c2;
        unsigned int *neighbor2 = graph_get_neighbors(graph_player, *position, &c2);
        *position = path[(*size_path) - 1];
        (*size_path)--;
        *last_dir = get_direction(old_position_player, *position);
        if (*position_player == *position_opponent)
        {
            unsigned int c1;
            unsigned int *neighbor1 = graph_get_neighbors(graph_player, *position, &c1);
            for (unsigned int i = 0; i < c1; i++)
            {
                if (is_in_direction(*position, neighbor1[i], *last_dir, 1))
                {
                    *position = neighbor1[i];
                    break;
                }
            }
            if (*position_player == *position_opponent)
            {
                if (neighbor2[0] != *position_opponent)
                {
                    *position = neighbor2[0];
                }
                else
                {
                    *position = neighbor2[1];
                }
            }
            free(neighbor1);
        }
        for (unsigned int i = 0; i < graph_player->num_objectives; i++)
        {
            if (*position == graph_player->objectives[i])
            {
                visited_objective[i] = 1;
            }
        }
        free(neighbor2);
    }
}

int wall_impact_path(unsigned int *path, unsigned int *size_path, struct edge_t edge1,
                     struct edge_t edge2)
{
    for (unsigned int i = 1; i < (*size_path); i++)
    {
        if ((path[i] == edge1.fr && path[i] == edge1.to) ||
            (path[i] == edge2.fr && path[i] == edge2.to))
        {
            return 1;
        }
    }
    return 0;
}

enum move_type_t action_type(struct edge_t *edge1, struct edge_t *edge2)
{
    int blocked = 0;
    if (*size_path_opponent <= 1 || *size_path_opponent >= graph_player->num_vertices)
    {
        blocked = fill_path(path_opponent, size_path_opponent, visited_objective_opponent,
                            position_opponent, origin_position_opponent, previous_dir_opponent);
    }

    if ((!blocked) && (nb_wall_placed < graph_player->num_edges / 16) &&
        (path_opponent[(*size_path_opponent) - 1] < graph_player->num_vertices))
    {
        unsigned int close_neighbor = get_close_neighbor(
            *position_opponent,
            path_opponent[(*size_path_opponent) - 1]); // nearest node toward opponent's target
        edge1->fr = close_neighbor;
        edge1->to = *position_opponent; // ensure the wall intersects the opponent path
        struct edge_t *edge_neighbors = malloc(4 * (sizeof(struct edge_t)));
        unsigned int nb_edge_neighbors = get_edge_neighbors(
            edge_neighbors,
            *edge1); // candidate edges to pair with the chosen wall segment
        if (nb_edge_neighbors == 0)
        {
            free(edge_neighbors);
            return MOVE;
        }
        // choose which wall to place
        int valid_wall = 0;
        int max_slower = 2;
        unsigned int *predicted_walled_path;
        for (unsigned int i = 0; i < nb_edge_neighbors; i++)
        {
            if (wall_impact_path(path_player, size_path_player, *edge1, edge_neighbors[i]) == 0)
            {
                *edge2 = edge_neighbors[i];
                unsigned int backup_direction1 = get_direction(edge1->fr, edge1->to);
                unsigned int backup_direction2 = get_direction(edge2->fr, edge2->to);
                graph_remove_edge(graph_player, *edge1);
                graph_remove_edge(graph_player, *edge2);
                predicted_walled_path = calloc(graph_player->num_vertices, sizeof(unsigned int));
                unsigned int size_predicted_walled_path = 0;
                int blocked_path_to_objective = fill_path(
                    predicted_walled_path, &size_predicted_walled_path, visited_objective_opponent,
                    position_opponent, origin_position_opponent, previous_dir_opponent);
                graph_add_edge_compressed(graph_player, *edge1, backup_direction1);
                graph_add_edge_compressed(graph_player, *edge2, backup_direction2);
                if (!blocked_path_to_objective &&
                    (int)predicted_walled_path[0] - (int)*size_path_opponent > max_slower)
                {
                    valid_wall = 1;
                    max_slower = predicted_walled_path[0] - (int)*size_path_opponent;
                }
                free(predicted_walled_path);
            }
        }
        if (valid_wall == 0)
        {
            free(edge_neighbors);
            edge1->to = 0;
            edge1->fr = 0;
            edge2->to = 0;
            edge2->fr = 0;
            return MOVE;
        }
        if (edge1->fr !=
            edge2->fr)
        { // verify wall segments share the same origin
            if (edge1->fr == edge2->to)
            {
                unsigned int tmp1 = edge2->to;
                edge2->to = edge2->fr;
                edge2->fr = tmp1;
            }
            else if (edge1->to == edge2->to)
            {
                unsigned int tmp2 = edge1->to; // == edge2->to
                edge1->to = edge1->fr;
                edge1->fr = tmp2;
                edge2->to = edge2->fr;
                edge2->fr = tmp2;
            }
            else if (edge1->to == edge2->fr)
            {
                unsigned int tmp3 = edge1->to;
                edge1->to = edge1->fr;
                edge1->fr = tmp3;
            }
        }
        free(edge_neighbors);
    }

    else
    {
        edge1->to = 0;
        edge1->fr = 0;
        edge2->to = 0;
        edge2->fr = 0;
        return MOVE;
    }
    return WALL;
}

struct move_t action(enum move_type_t act_t, struct edge_t edge1, struct edge_t edge2)
{
    if (act_t == MOVE)
    {
        if (*size_path_player <= 1 || *size_path_player >= graph_player->num_vertices)
        {
            fill_path(path_player, size_path_player, visited_objective_player, position_player,
                      origin_position_player, previous_dir_player);
        }
        if (*position_opponent == path_player[*(size_path_player)-1])
        {
            fill_path(path_player, size_path_player, visited_objective_player, position_player,
                      origin_position_player, previous_dir_player);
        }
        follow_path(path_player, size_path_player, position_player, previous_dir_player,
                    visited_objective_player);
    }
    else if (act_t == WALL)
    {
        nb_wall_placed++;
        graph_remove_edge(graph_player, edge1);
        graph_remove_edge(graph_player, edge2);
        fill_path(path_opponent, size_path_opponent, visited_objective_opponent, position_opponent,
                  origin_position_opponent, previous_dir_opponent);
        // removed edges do not affect the player's path, so no update needed here
    }
    struct move_t next_move = {
        .c = color_player, .t = act_t, .m = *position_player, .e = {edge1, edge2}};
    return next_move;
}
