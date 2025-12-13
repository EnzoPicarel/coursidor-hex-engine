#include "heldkarp_player.h"
#include "gen_graph.h"
#include "player.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

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

static unsigned int *visited_objective_opponent;

static vertex_t *position_player;
static vertex_t *position_opponent;

static vertex_t *path_player;
static vertex_t *path_opponent;

static unsigned int *size_path_player;
static unsigned int *size_path_opponent;

static enum dir_t *previous_dir_player;
static enum dir_t *previous_dir_opponent;

static unsigned int **cost_matrix;
static unsigned int *optimal_order;
static unsigned int index_global_path = 1;

// PLAYING

char const *get_player_name()
{
    return "JEAN HELDKARP";
}

void initialize(unsigned int player_id, struct graph_t *graph)
{
    graph_player = graph;

    visited_objective_opponent = calloc(graph->num_objectives, sizeof(unsigned int));

    color_player = player_id;
    if (color_player == BLACK)
        color_opponent = WHITE;
    else
        color_opponent = BLACK;
    position_player = malloc(sizeof(vertex_t));
    *position_player = graph_player->start[color_player];
    path_player = calloc(graph_player->num_vertices, sizeof(unsigned int));
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

    cost_matrix = compute_cost_matrix(origin_position_player, graph_player->objectives,
                                      graph_player->num_objectives, NO_EDGE);
    optimal_order = held_karp_tsp(cost_matrix, graph_player->num_objectives);
    /*
    printf("optimal order\n");
    for (unsigned int i = 1; i< graph_player->num_objectives + 1; i++) {
        printf("%u, ", graph_player->objectives[optimal_order[i]-1]);
    }
    printf("\n");
    */
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
        fill_path_player(path_player, size_path_player, position_player, origin_position_player,
                         previous_dir_player);
        fill_path_opponent(path_opponent, size_path_opponent, position_opponent,
                           origin_position_opponent, previous_dir_opponent);
        follow_path(path_player, size_path_player, position_player, previous_dir_player,
                    color_player);
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
            fill_path_player(path_player, size_path_player, position_player, origin_position_player,
                             previous_dir_player); // refresh path after wall placement
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
        if (*size_path_opponent <= 0)
        {
            fill_path_opponent(path_opponent, size_path_opponent, position_opponent,
                               origin_position_opponent, previous_dir_opponent);
        }
        if (*position_opponent == path_opponent[(*size_path_opponent) - 1])
        {
            (*size_path_opponent)--;
        }
        else
        {
            fill_path_opponent(path_opponent, size_path_opponent, position_opponent,
                               origin_position_opponent, previous_dir_opponent);
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
    unsigned int nb_obj = graph_player->num_objectives;
    graph_destroy(graph_player);
    free(position_player);
    free(position_opponent);
    free(path_player);
    free(path_opponent);
    free(previous_dir_opponent);
    free(previous_dir_player);
    free(size_path_player);
    free(size_path_opponent);
    free(visited_objective_opponent);
    if (cost_matrix != NULL)
    {
        for (unsigned i = 0; i <= nb_obj; i++)
        {
            free(cost_matrix[i]);
        }
        free(cost_matrix);
        cost_matrix = NULL;
    }

    if (optimal_order != NULL)
    {
        free(optimal_order);
        optimal_order = NULL;
    }
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
                previous_dir[y] = get_direction(x, y); // Update direction dynamiquement
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
        vertex_t curr = (vertex_t)(-1);
        for (int k = graph_player->t->p[v]; k < graph_player->t->p[v + 1]; k++)
        {
            if ((enum dir_t)graph_player->t->data[k] == dir)
            {
                curr = graph_player->t->i[k];
                break;
            }
        }
        if (curr == (vertex_t)(-1))
            continue; // no edge that way

        // record the 1-step move (unless it lands on the opponent)
        if (curr != *position_opponent)
        {
            tab[n] = curr;
            dist[n] = 1;
            n++;
        }

        // determine how far we can dash in 'dir'
        unsigned int maxd = max_length_move(before, dir);
        vertex_t prev = curr;

        // chain steps 2 and 3
        for (unsigned int step = 2; step <= maxd; step++)
        {
            vertex_t next = (vertex_t)(-1);
            for (int k = graph_player->t->p[prev]; k < graph_player->t->p[prev + 1]; k++)
            {
                if ((enum dir_t)graph_player->t->data[k] == dir)
                {
                    next = graph_player->t->i[k];
                    break;
                }
            }
            if (next == (vertex_t)(-1))
                break; // no further edge

            // record this step if it doesn't land on the opponent
            if (next != *position_opponent)
            {
                tab[n] = next;
                dist[n] = step;
                n++;
            }
            prev = next;
        }
    }

    // sentinel for “no more successors”
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

// Usage example:
// struct hex_pos *index_to_hex = malloc(num_vertices * sizeof(struct hex_pos));
// build_index_to_hex_table(index_to_hex, m);
// hex_pos pos_i = index_to_hex[i]; // to get coordinates of vertex i

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
    unsigned int close_neighbor = neighbors[0]; // default

    for (unsigned int i = 0; i < count; i++)
    {
        unsigned int neighbor = neighbors[i];
        struct hex_pos neigh_pos = index_to_hex[neighbor];
        int nvq = neigh_pos.q - start_pos.q;
        int nvr = neigh_pos.r - start_pos.r;
        // Scalar product calcul
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

int minimum(int a, int b) { return a < b ? a : b; }

int maximum(int a, int b) { return a > b ? a : b; }

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
    // Returns the direction to go from a to b
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

    for (unsigned int j = 0; j < orig[0]; j++)
    {
        dest[j] = orig[j];
    }
}

int fill_path_opponent(unsigned int *best_path, unsigned int *size_path, unsigned int *position,
                       unsigned int initial_start, enum dir_t *last_dir)
{

    int back_to_base = 1;
    int blocked_path_to_objective = 0;
    best_path[0] = graph_player->num_vertices;
    for (unsigned int i = 0; i < graph_player->num_objectives; i++)
    {
        if (visited_objective_opponent[i] == 0)
        {
            back_to_base = 0;
            unsigned int *path_proposition =
                a_star(*position, graph_player->objectives[i], *last_dir);
            if (path_proposition[0] == 1)
            { // also check if remaining objectives are reachable
                blocked_path_to_objective = 1;
            }
            else if (path_proposition[0] < best_path[0])
            {
                copy_a_star_result(path_proposition, best_path);
            }
            free(path_proposition);
        }
    }

    unsigned int *path_proposition = a_star(*position, initial_start, *last_dir);
    if (path_proposition[0] == 1)
    { // check if we can return to the starting point
        blocked_path_to_objective = 1;
    }
    if (back_to_base)
    {
        copy_a_star_result(path_proposition, best_path);
    }
    free(path_proposition);
    *size_path = best_path[0];
    return blocked_path_to_objective;
}

void fill_path_player(unsigned int *best_path, unsigned int *size_path, unsigned int *position,
                      unsigned int initial_start, enum dir_t *last_dir)
{
    if (index_global_path > graph_player->num_objectives)
    {
        unsigned int *retour = a_star(*position, initial_start, *last_dir);
        copy_a_star_result(retour, best_path);
        free(retour);
    }
    else
    {
        unsigned int *p = a_star(
            *position, graph_player->objectives[optimal_order[index_global_path] - 1], *last_dir);
        copy_a_star_result(p, best_path);
        free(p);
    }
    *size_path = best_path[0];
}

void follow_path(unsigned int *path, unsigned int *size_path, unsigned int *position,
                 enum dir_t *last_dir, unsigned int color)
{
    vertex_t old_position_player = *position;
    if (color == color_opponent)
    {
        if (*size_path <= 0)
        {
            fill_path_opponent(path, size_path, position, origin_position_opponent, last_dir);
        }
    }
    else
    {
        if (*size_path <= 0)
        {
            fill_path_player(path, size_path, position, origin_position_player, last_dir);
        }
    }

    // Get the next target position
    vertex_t target = path[(*size_path) - 1];

    // Use the same logic as outgoing_edge_list to check reachability
    bool legal_move = false;
    vertex_t *reachable = calloc(10, sizeof(unsigned int));
    unsigned int *distances = calloc(10, sizeof(unsigned int));

    // Fill reachable positions based on movement rules
    outgoing_edge_list(*last_dir, old_position_player, reachable, distances);

    // Check if target is in the reachable positions
    for (int i = 0; i < 10 && reachable[i] != INF; i++)
    {
        if (reachable[i] == target)
        {
            legal_move = true;
            break;
        }
    }

    free(reachable);
    free(distances);

    if (!legal_move)
    {
        // If the move isn't legal, recalculate path with smaller steps
        if (color == color_opponent)
        {
            fill_path_opponent(path, size_path, position, origin_position_opponent, last_dir);
        }
        else
        {
            fill_path_player(path, size_path, position, origin_position_player, last_dir);
        }
        target = path[(*size_path) - 1]; // Get the new next step
    }

    // Now make the move

    *position = target;
    (*size_path)--;
    *last_dir = get_direction(old_position_player, *position);

    // Check if we've reached an objective
    for (unsigned int i = 0; i < graph_player->num_objectives; i++)
    {
        if (*position == graph_player->objectives[i])
        {
            if (color == color_opponent)
            {
                visited_objective_opponent[i] = 1;
            }
            else
            {
                index_global_path += 1;
            }
        }
    }
}

int wall_impact_path(unsigned int *path, unsigned int *size_path, struct edge_t edge1,
                     struct edge_t edge2)
{
    for (unsigned int i = 1; i < (*size_path); i++)
    {
        if (path[i] == edge1.fr || path[i] == edge1.to || path[i] == edge2.fr ||
            path[i] == edge2.to)
        {
            return 1;
        }
    }
    return 0;
}

enum move_type_t action_type(struct edge_t *edge1, struct edge_t *edge2)
{
    if (*size_path_opponent <= 0)
    {
        fill_path_opponent(path_opponent, size_path_opponent, position_opponent,
                           origin_position_opponent, previous_dir_opponent);
    }
    for (unsigned int i = 0; i < *size_path_opponent; i++)
    {
    }
    unsigned int close_neighbor = get_close_neighbor(
        *position_opponent,
        path_opponent[(*size_path_opponent) - 1]); // nearest node toward opponent's target
    edge1->fr = close_neighbor;
    edge1->to = *position_opponent; // ensure wall intersects the opponent path
    struct edge_t *edge_neighbors = malloc(4 * (sizeof(struct edge_t)));
    unsigned int nb_edge_neighbors = get_edge_neighbors(edge_neighbors,
                                                        *edge1); // candidate wall pairings
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
            graph_remove_edge(graph_player, *edge1);
            graph_remove_edge(graph_player, *edge2);
            predicted_walled_path = calloc(graph_player->num_vertices, sizeof(unsigned int));
            unsigned int size_predicted_walled_path = 0;
            int blocked_path_to_objective = fill_path_opponent(
                predicted_walled_path, &size_predicted_walled_path, position_opponent,
                origin_position_opponent, previous_dir_opponent);
            graph_add_edge(graph_player, *edge1, get_direction(edge1->fr, edge1->to));
            graph_add_edge(graph_player, *edge2, get_direction(edge2->fr, edge2->to));
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
    return WALL;
}

struct move_t action(enum move_type_t act_t, struct edge_t edge1, struct edge_t edge2)
{
    if (act_t == MOVE)
    {
        if (*size_path_player == 0)
        {
            fill_path_player(path_player, size_path_player, position_player, origin_position_player,
                             previous_dir_player);
        }
        if (*position_opponent == path_player[*(size_path_player)-1])
        {
            fill_path_player(path_player, size_path_player, position_player, origin_position_player,
                             previous_dir_player);
        }
        follow_path(path_player, size_path_player, position_player, previous_dir_player,
                    color_player);
    }
    else if (act_t == WALL)
    {
        graph_remove_edge(graph_player, edge1);
        graph_remove_edge(graph_player, edge2);
        fill_path_opponent(path_opponent, size_path_opponent, position_opponent,
                           origin_position_opponent, previous_dir_opponent);
        // removed edges do not affect the player's path, so no update needed here
    }
    struct move_t next_move = {
        .c = color_player, .t = act_t, .m = *position_player, .e = {edge1, edge2}};
    return next_move;
}

unsigned int **compute_cost_matrix(unsigned int start, unsigned int *objectives, unsigned int n_obj,
                                   enum dir_t init_dir)
{
    unsigned int **dist = malloc((n_obj + 1) * sizeof(*dist));
    for (unsigned i = 0; i <= n_obj; i++)
    {
        dist[i] = malloc((n_obj + 1) * sizeof(*dist[i]));
    }
    unsigned int *pts = malloc((n_obj + 1) * sizeof(*pts));
    pts[0] = start;
    for (unsigned i = 1; i <= n_obj; i++)
        pts[i] = objectives[i - 1];

    for (unsigned i = 0; i <= n_obj; i++)
    {
        for (unsigned j = 0; j <= n_obj; j++)
        {
            if (i == j)
            {
                dist[i][j] = 0;
            }
            else
            {
                unsigned int *path = a_star(pts[i], pts[j], init_dir);
                dist[i][j] = (path[0] > 0 ? path[0] - 1 : INF);
                free(path);
            }
        }
    }
    free(pts);
    return dist;
}

// Returns an array of indices (1…n) in the visit order.
// tour[0] = n (the size), then tour[1]…tour[n] reference indices in pts[]
unsigned int *held_karp_tsp(unsigned int **dist, unsigned int n)
{
    const unsigned FULL = 1u << n;
    unsigned int **dp = malloc(FULL * sizeof(*dp));
    unsigned int **parent = malloc(FULL * sizeof(*parent));
    for (unsigned mask = 0; mask < FULL; mask++)
    {
        dp[mask] = malloc(n * sizeof(*dp[mask]));
        parent[mask] = malloc(n * sizeof(*parent[mask]));
        for (unsigned j = 0; j < n; j++)
        {
            dp[mask][j] = INF;
            parent[mask][j] = n; // marqueur “pas de parent”
        }
    }

    // Initialization
    for (unsigned j = 0; j < n; j++)
    {
        unsigned m = 1u << j;
        dp[m][j] = dist[0][j + 1];
    }

    // Loop over all subsets
    for (unsigned mask = 1; mask < FULL; mask++)
    {
        for (unsigned j = 0; j < n; j++)
        {
            if (!(mask & (1u << j)))
                continue;
            unsigned prev = mask ^ (1u << j);
            if (prev == 0)
                continue;
            for (unsigned k = 0; k < n; k++)
            {
                if (!(prev & (1u << k)))
                    continue;
                unsigned cost = dp[prev][k] + dist[k + 1][j + 1];
                if (cost < dp[mask][j])
                {
                    dp[mask][j] = cost;
                    parent[mask][j] = k;
                }
            }
        }
    }

    // choose the best path
    unsigned final_mask = FULL - 1;
    unsigned best_j = 0;
    unsigned best_cost = INF;
    for (unsigned j = 0; j < n; j++)
    {
        unsigned int total_cost = dp[final_mask][j] + dist[j + 1][0];
        if (total_cost < best_cost)
        {
            best_cost = dp[final_mask][j];
            best_j = j;
        }
    }

    // Reconstruct the path
    unsigned int *tour = malloc((n + 1) * sizeof(*tour));
    tour[0] = n;
    unsigned curr_j = best_j;
    unsigned mask = final_mask;
    for (unsigned pos = n; pos >= 1; pos--)
    {
        tour[pos] = curr_j + 1;
        unsigned prev_k = parent[mask][curr_j];
        mask ^= (1u << curr_j);
        curr_j = prev_k;
    }

    for (unsigned mask2 = 0; mask2 < FULL; mask2++)
    {
        free(dp[mask2]);
        free(parent[mask2]);
    }
    free(dp);
    free(parent);

    return tour;
}
