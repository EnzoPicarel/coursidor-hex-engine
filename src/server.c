#include "gen_graph.h"
#include "move.h"
#include "player.h"
#include <dlfcn.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void initialize_player(void *handle, unsigned int player_id, struct graph_t *graph) {
    void (*initialize)(unsigned int, struct graph_t *);
    initialize = (void (*)(unsigned int, struct graph_t *))dlsym(handle, "initialize");

    if (initialize) {
        initialize(player_id, graph);
    } else {
        fprintf(stderr, "Error: could not find initialize function.\n");
        exit(EXIT_FAILURE);
    }
}

char const *get_player_name(void *player_handle) {
    char const *(*name)();
    name = (char const *(*)())dlsym(player_handle, "get_player_name");
    if (name) {
        return name();
    } else {
        fprintf(stderr, "Error: could not find get_player_name function.\n");
        exit(EXIT_FAILURE);
    }
}

unsigned int get_position_player(void *handle) {
    unsigned int (*get_position)();
    get_position = (unsigned int (*)())dlsym(handle, "get_position_player");
    return get_position();
}

struct move_t play_move(void *handle, struct move_t previous_move) {
    struct move_t (*play)(const struct move_t);
    play = (struct move_t(*)(const struct move_t))dlsym(handle, "play");

    if (play) {
        return play(previous_move);
    } else {
        fprintf(stderr, "Error: could not find play function.\n");
        exit(EXIT_FAILURE);
    }
}

void finalize_player(void *handle) {
    void (*finalize)();
    finalize = (void (*)())dlsym(handle, "finalize");

    if (finalize) {
        finalize();
    } else {
        fprintf(stderr, "Error: could not find finalize function.\n");
        exit(EXIT_FAILURE);
    }
}

int is_game_over(int **obj, struct move_t pos, struct move_t initial_pos, int player,
                 unsigned int num_obj) {
    int nb = 1;
    for (unsigned int j = 0; j < num_obj; j++) {
        if (obj[player][j] == 0) {
            nb = 0;
            break;
        }
    }
    if ((nb == 1) && (pos.m == initial_pos.m))
        return 1;

    return -1;
}

int random_player() {
    return (rand() % 2);
}

void *switch_player(void *current, void *p1, void *p2) {
    if (current == p1)
        return p2;
    return p1;
}

int on_obj(struct graph_t *t, struct move_t move) {
    for (unsigned int i = 0; i < t->num_objectives; i++) {
        if (t->objectives[i] == move.m) {
            return i;
        }
    }
    return -1;
}

enum dir_t *get_direction_30(enum dir_t direction) {
    enum dir_t *return_dir = malloc(sizeof(enum dir_t) * 2);

    switch (direction) {
    case NW:
        return_dir[0] = W;
        return_dir[1] = NE;
        break;
    case NE:
        return_dir[0] = NW;
        return_dir[1] = E;
        break;
    case E:
        return_dir[0] = NE;
        return_dir[1] = SE;
        break;
    case SE:
        return_dir[0] = E;
        return_dir[1] = SW;
        break;
    case SW:
        return_dir[0] = SE;
        return_dir[1] = W;
        break;
    case W:
        return_dir[0] = SW;
        return_dir[1] = NW;
        break;
    default:
        return_dir[0] = NO_EDGE;
        return_dir[1] = NO_EDGE;
    }
    return return_dir;
}

int is_in_array(vertex_t *arr, unsigned int len, vertex_t v) {
    for (unsigned int i = 0; i < len; i++) {
        if (arr[i] == v)
            return 1;
    }
    return 0;
}

void get_reachable_vertices(struct graph_t *graph, vertex_t current_pos, enum dir_t prev_dir,
                            vertex_t *reachable, unsigned int *num_reachable) {
    *num_reachable = 0;
    unsigned int count;
    unsigned int *neighbors = graph_get_neighbors(graph, current_pos, &count);

    // Premier mouvement ou pas de direction précédente
    if (prev_dir == NO_EDGE) {
        for (unsigned int i = 0; i < count; i++) {
            enum dir_t dir = gsl_spmatrix_uint_get(graph->t, current_pos, neighbors[i]);
            if (dir != WALL_DIR && dir != NO_EDGE) {
                reachable[(*num_reachable)++] = neighbors[i];
            }
        }
        free(neighbors);
        return;
    }

    // Obtenir les directions à 30°
    enum dir_t *dirs_30 = get_direction_30(prev_dir);

    // Parcourir tous les voisins du premier niveau
    for (unsigned int i = 0; i < count; i++) {
        enum dir_t dir = gsl_spmatrix_uint_get(graph->t, current_pos, neighbors[i]);
        if (dir == WALL_DIR || dir == NO_EDGE)
            continue;

        vertex_t next = neighbors[i];

        // Si même direction
        if (dir == prev_dir) {
            // Ajouter le voisin direct
            if (!is_in_array(reachable, *num_reachable, next)) {
                reachable[(*num_reachable)++] = next;
            }

            // Explorer jusqu'à 2 cases supplémentaires dans la même direction
            vertex_t curr = next;
            for (int step = 0; step < 2 && curr != (vertex_t)-1; step++) {
                unsigned int next_count;
                unsigned int *next_neighbors = graph_get_neighbors(graph, curr, &next_count);
                int found = 0;

                for (unsigned int j = 0; j < next_count; j++) {
                    enum dir_t next_dir = gsl_spmatrix_uint_get(graph->t, curr, next_neighbors[j]);
                    if (next_dir == dir) {
                        vertex_t next_vertex = next_neighbors[j];
                        if (!is_in_array(reachable, *num_reachable, next_vertex)) {
                            reachable[(*num_reachable)++] = next_vertex;
                            curr = next_vertex;
                            found = 1;
                            break;
                        }
                    }
                }
                free(next_neighbors);
                if (!found)
                    break;
            }
        }
        // Si direction à 30°
        else if (dir == dirs_30[0] || dir == dirs_30[1]) {
            // Ajouter le voisin direct
            if (!is_in_array(reachable, *num_reachable, next)) {
                reachable[(*num_reachable)++] = next;
            }

            // Explorer une case supplémentaire dans la direction à 30°
            vertex_t curr = next;
            unsigned int next_count;
            unsigned int *next_neighbors = graph_get_neighbors(graph, curr, &next_count);

            for (unsigned int j = 0; j < next_count; j++) {
                enum dir_t next_dir = gsl_spmatrix_uint_get(graph->t, curr, next_neighbors[j]);
                if (next_dir == dir) {
                    vertex_t next_vertex = next_neighbors[j];
                    if (!is_in_array(reachable, *num_reachable, next_vertex)) {
                        reachable[(*num_reachable)++] = next_vertex;
                        break;
                    }
                }
            }
            free(next_neighbors);
        }
    }

    free(neighbors);
    free(dirs_30);
}
enum dir_t get_direction_between_vertices(struct graph_t *graph, vertex_t start, vertex_t end) {
    if (start == end)
        return NO_EDGE;

    vertex_t *queue = malloc(graph->num_vertices * sizeof(vertex_t));
    int *visited = calloc(graph->num_vertices, sizeof(int));
    enum dir_t *first_dir = malloc(graph->num_vertices * sizeof(enum dir_t));
    int front = 0, rear = 0;

    // Initialiser la file avec le sommet de intiale qui est le centre du graphe
    queue[rear++] = start;
    visited[start] = 1;
    first_dir[start] = NO_EDGE;

    enum dir_t result = NO_EDGE;

    while (front < rear) {
        vertex_t current = queue[front++];

        // Si on a trouvé la destination
        if (current == end) {
            result = first_dir[current];
            break;
        }

        // Explorer les voisins
        unsigned int count;
        unsigned int *neighbors = graph_get_neighbors(graph, current, &count);

        for (unsigned int i = 0; i < count; i++) {
            vertex_t next = neighbors[i];
            if (!visited[next]) {
                enum dir_t dir = gsl_spmatrix_uint_get(graph->t, current, next);
                if (dir != WALL_DIR && dir != NO_EDGE) {
                    queue[rear++] = next;
                    visited[next] = 1;
                    // Si c'est le premier pas, sauvegarder la direction
                    first_dir[next] = (current == start) ? dir : first_dir[current];
                }
            }
        }
        free(neighbors);
    }

    free(queue);
    free(visited);
    free(first_dir);

    return result;
}

static int can_reach_all_vertices(struct graph_t *graph, vertex_t start) {
    int *visited = calloc(graph->num_vertices, sizeof(int));
    vertex_t *queue = malloc(graph->num_vertices * sizeof(vertex_t));
    int front = 0, rear = 0;
    unsigned int count = 0;

    // Initialisation file pour parcours largeur
    queue[rear++] = start;
    visited[start] = 1;
    count++;

    while (front < rear) {
        vertex_t current = queue[front++];
        unsigned int neighbor_count;
        unsigned int *neighbors = graph_get_neighbors(graph, current, &neighbor_count);

        for (unsigned int i = 0; i < neighbor_count; i++) {
            vertex_t next = neighbors[i];
            if (!visited[next]) {
                enum dir_t dir = gsl_spmatrix_uint_get(graph->t, current, next);
                if (dir != WALL_DIR && dir != NO_EDGE) {
                    queue[rear++] = next;
                    visited[next] = 1;
                    count++;
                }
            }
        }
        free(neighbors);
    }

    free(queue);
    free(visited);

    // If count equals number of vertices, all vertices are reachable then return 1
    if (count == graph->num_vertices) {
        return 1;
    }
    return -1;
}

enum enum_possible {
    MOVE_ALLOWED = 1,
    ERROR_NO_TYPE_MOVE = 0,
    ERROR_INVALID_VERTEX = -1,
    ERROR_PLAYER_DONT_MOVE = -2,
    ERROR_GOING_OPPONENT = -3,
    ERROR_WALL = -4
};

int possible_move(struct graph_t *graph, struct move_t move, struct move_t *position_player, int i1,
                  struct move_t *position_player_other, int i2) {
    if (move.t == MOVE) {
        if (move.m == position_player[i1].m)
            return ERROR_PLAYER_DONT_MOVE;
        /*
        printf("\n=== DEBUG INFO ===\n");
        printf("Current position: %d\n", position_player[i1].m);
        printf("Target position: %d\n", move.m);
        */
        // Check opponent position
        if (move.m == position_player_other[i2].m) {
            printf("DEBUG: Trying to move to opponent position\n");
            return ERROR_GOING_OPPONENT;
        }

        // Utilisez la fonction get_direction_between_vertices pour obtenir la direction
        /*
        printf("Previous position: %d\n", prev);
        printf("Current position: %d\n", curr);
        printf("Previous direction: %d\n", prev_dir);
        */
        unsigned int c;
        unsigned int *neighbor1 = graph_get_neighbors(graph, position_player[i1].m, &c);
        for (unsigned int i = 0; i < c; i++) {
            if (neighbor1[i] == move.m) {
                free(neighbor1);
                return MOVE_ALLOWED;
            }
        }
        free(neighbor1);
        if (i1 == 0)
            return ERROR_INVALID_VERTEX; // cas ou c'est le premier move et que la destination n'est
                                         // pas dans les sommets adjaçants

        vertex_t curr = position_player[i1].m;
        vertex_t prev = position_player[i1 - 1].m;
        enum dir_t prev_dir = get_direction_between_vertices(graph, prev, curr);

        // Get all reachable vertices from current position
        vertex_t reachable[graph->num_vertices];
        unsigned int num_reachable;
        get_reachable_vertices(graph, curr, prev_dir, reachable, &num_reachable);

        printf("Reachable vertices: ");
        for (unsigned int i = 0; i < num_reachable; i++) {
            printf("%d ", reachable[i]);
        }
        printf("\n=================\n");

        // Check if target is in reachable vertices
        for (unsigned int i = 0; i < num_reachable; i++) {
            if (reachable[i] == move.m) {
                return MOVE_ALLOWED;
            }
        }

        // Check if the player moved behind the opposite player by checking direction
        enum dir_t direction = get_direction_between_vertices(graph, curr, move.m);
        if (direction ==
            get_direction_between_vertices(graph, position_player_other[i2].m, move.m)) {
            return MOVE_ALLOWED;
        }

        return ERROR_INVALID_VERTEX;
    } else if (move.t == WALL) {
        graph_remove_edge(graph, move.e[0]);
        graph_remove_edge(graph, move.e[1]);

        // MOVE_ALLOWED if wall legal, -1 if not
        if (can_reach_all_vertices(graph, graph->num_vertices / 2) == 1) {
            return MOVE_ALLOWED;
        }
        return ERROR_WALL;
    }
    printf("NO TYPE MOVE\n");
    return ERROR_NO_TYPE_MOVE;
}
int main(int argc, char *argv[]) {
    srand(time(NULL));

    int size = 0;
    enum graph_type_t type = TRIANGULAR;

    type++;

    int max_round = 500;
    int opt;

    unsigned int num_objectives = 6;

    while ((opt = getopt(argc, argv, "m:t:M:O:")) != -1) {
        switch (opt) {
        case 'm':
            size = atoi(optarg);
            break;
        case 't':
            if (strcmp(optarg, "T") == 0) {
                type = TRIANGULAR;
            } else if (strcmp(optarg, "C") == 0) {
                type = CYCLIC;
            } else {
                fprintf(stderr, "Invalid graph type. Use 'T' for triangular or 'C' for cyclic.\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 'M':
            max_round = atoi(optarg) * 2;
            break;
        case 'O':
            num_objectives = atoi(optarg);
            break;
        default:
            fprintf(
                stderr,
                "Usage: %s [-m width] [-t type] [-M max round] [-O num objectives] <player1.so> "
                "<player2.so>\n",
                argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (argc - optind < 2) {
        fprintf(stderr,
                "Usage: %s [-m width] [-t type] [-M max round] [-O num objectives] <player1.so> "
                "<player2.so>\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    void *player1_handle = dlopen(argv[optind], RTLD_LAZY);
    void *player2_handle = dlopen(argv[optind + 1], RTLD_LAZY);
    if (!player1_handle || !player2_handle) {
        fprintf(stderr, "Error loading player shared libraries.\n");
        return EXIT_FAILURE;
    }

    int **spot = malloc(2 * sizeof(int *));
    for (int i = 0; i < 2; i++) {
        spot[i] = malloc(num_objectives * sizeof(int));
    }
    for (unsigned int i = 0; i < num_objectives; i++) {
        spot[BLACK][i] = 0;
        spot[WHITE][i] = 0;
    }

    unsigned int num_vertices = 3 * size * size - 3 * size + 1;

    // Initialize game graph here
    unsigned int tab[num_objectives];
    for (unsigned int i = 0; i < num_objectives; i++) {
        tab[i] = i;
    }

    if (num_objectives % 2 == 1) {
        tab[num_objectives - 1] = (num_vertices - 1) / 2;
        for (unsigned int i = 0; i < num_objectives - 1;) {
            unsigned int rand_vert;
            do {
                rand_vert = rand() % (num_vertices - 1);
            } while (rand_vert <= 0 || rand_vert == tab[num_objectives - 1] ||
                     is_in_array(tab, num_objectives, rand_vert) == 1 ||
                     rand_vert == num_vertices - 1 || rand_vert == 0);
            tab[i++] = rand_vert;
            tab[i++] = num_vertices - 1 - rand_vert;
        }
    } else {
        for (unsigned int i = 0; i < num_objectives - 1;) {
            unsigned int rand_vert;
            do {
                rand_vert = rand() % (num_vertices - 1);
            } while (rand_vert <= 0 || rand_vert == (num_vertices - 1) / 2 ||
                     is_in_array(tab, num_objectives, rand_vert) == 1 ||
                     rand_vert == num_vertices - 1 || rand_vert == 0);
            tab[i++] = rand_vert;
            tab[i++] = num_vertices - 1 - rand_vert;
        }
    }

    // Create the graphs
    struct graph_t *graph = graph_create_triangular(size);
    struct graph_t *graph_p1 = graph_create_triangular(size);
    struct graph_t *graph_p2 = graph_create_triangular(size);

    graph_add_objectives(graph, num_objectives, tab);
    graph_add_objectives(graph_p1, num_objectives, tab);
    graph_add_objectives(graph_p2, num_objectives, tab);

    // print objectives positions
    printf("Objectives positions: ");
    for (unsigned int i = 0; i < graph->num_objectives; i++) {
        printf("%d ", graph->objectives[i]);
    }
    printf("\n");

    struct move_t *position_player_handle1 = malloc(sizeof(struct move_t) * max_round);
    int i1 = 0;

    struct move_t *position_player_handle2 = malloc(sizeof(struct move_t) * max_round);
    int i2 = 0;

    int first_player = random_player();

    int player1_id, player2_id;

    struct edge_t edge_start1 = {.fr = 0, .to = 1};
    struct edge_t edge_start2 = {.fr = 1, .to = 2};

    struct move_t move_start;
    move_start.t = NO_TYPE;
    move_start.e[0] = edge_start1;
    move_start.e[1] = edge_start2;

    void *current_player;
    if (first_player == 0) {
        player1_id = BLACK;
        player2_id = WHITE;

        initialize_player(player1_handle, BLACK, graph_p1);
        move_start.m = graph->start[BLACK];
        position_player_handle1[0] = move_start;
        printf("PLAYER 0 is %s in pos %d\n", get_player_name(player1_handle),
               position_player_handle1[0].m);

        initialize_player(player2_handle, WHITE, graph_p2);
        move_start.m = graph->start[WHITE];
        position_player_handle2[0] = move_start;
        printf("PLAYER 1 is %s in pos %d\n", get_player_name(player2_handle),
               position_player_handle2[0].m);

        current_player = player1_handle;

    } else {
        player1_id = WHITE;
        player2_id = BLACK;

        initialize_player(player2_handle, BLACK, graph_p1);
        move_start.m = graph->start[BLACK];
        position_player_handle2[0] = move_start;
        printf("PLAYER 0 is %s in pos %d\n", get_player_name(player2_handle),
               position_player_handle2[0].m);

        initialize_player(player1_handle, WHITE, graph_p2);
        move_start.m = graph->start[WHITE];
        position_player_handle1[0] = move_start;
        printf("PLAYER 1 is %s in pos %d\n", get_player_name(player1_handle),
               position_player_handle1[0].m);

        current_player = player2_handle;
    }

    printf("%s starts\n", get_player_name(current_player));

    struct edge_t edge1 = {.fr = 0, .to = 1};
    struct edge_t edge2 = {.fr = 1, .to = 2};

    struct move_t current_move;
    current_move.t = NO_TYPE;
    current_move.c = NO_COLOR;
    current_move.m = 10000;
    current_move.e[0] = edge1;
    current_move.e[1] = edge2;

    printf("====================START====================\n");
    int i = 0;
    int game = 1;
    while (i < max_round && game) {
        current_move = play_move(current_player, current_move);
        if (current_player == player1_handle)
            printf("type : %d, player : %d, on %d ", current_move.t, current_move.c,
                   position_player_handle1[i1].m);
        else
            printf("type : %d, player : %d, on %d ", current_move.t, current_move.c,
                   position_player_handle2[i2].m);

        if (current_player == player1_handle) {
            enum enum_possible kind_move1 = possible_move(
                graph, current_move, position_player_handle1, i1, position_player_handle2, i2);
            switch (kind_move1) {
            case ERROR_INVALID_VERTEX:
                printf("\n\nILLEGAL MOVE FROM %s, position: %d try to go to: %d.\n",
                       get_player_name(current_player), position_player_handle1[i1].m,
                       current_move.m);
                game = 0;
                break;
            case ERROR_WALL:
                printf(
                    "\n\nILLEGAL MOVE FROM %s, try to put a wall that block the graph : %d -> %d "
                    "and %d -> %d\n",
                    get_player_name(current_player), current_move.e[0].fr, current_move.e[0].to,
                    current_move.e[1].fr, current_move.e[1].to);
                game = 0;
                break;
            case ERROR_PLAYER_DONT_MOVE:
                printf("\n\nILLEGAL MOVE FROM %s, doesn't move vertex from %d -> %d\n",
                       get_player_name(current_player), current_move.m,
                       position_player_handle1[i1].m);
                game = 0;
                break;
            case ERROR_GOING_OPPONENT:
                printf("\n\nILLEGAL MOVE FROM %s, try to go on opponent player %d = %d\n",
                       get_player_name(current_player), current_move.m,
                       position_player_handle2[i2].m);
                game = 0;
                break;
            default:
                break;
            }
        } else {
            enum enum_possible kind_move2 = possible_move(
                graph, current_move, position_player_handle2, i2, position_player_handle1, i1);
            switch (kind_move2) {
            case ERROR_INVALID_VERTEX:
                printf("\n\nILLEGAL MOVE FROM %s, position: %d try to go to: %d.\n",
                       get_player_name(current_player), position_player_handle2[i2].m,
                       current_move.m);
                game = 0;
                break;
            case ERROR_WALL:
                printf(
                    "\n\nILLEGAL MOVE FROM %s, try to put a wall that block the graph : %d -> %d "
                    "and %d -> %d\n",
                    get_player_name(current_player), current_move.e[0].fr, current_move.e[0].to,
                    current_move.e[1].fr, current_move.e[1].to);
                game = 0;
                break;
            case ERROR_PLAYER_DONT_MOVE:
                printf("\n\nILLEGAL MOVE FROM %s, doesn't move vertex from %d -> %d\n",
                       get_player_name(current_player), current_move.m,
                       position_player_handle2[i2].m);
                game = 0;
                break;
            case ERROR_GOING_OPPONENT:
                printf("\n\nILLEGAL MOVE FROM %s, try to go on opponent player %d = %d\n",
                       get_player_name(current_player), current_move.m,
                       position_player_handle1[i1].m);
                game = 0;
                break;
            default:
                break;
            }
        }

        if (current_move.t == MOVE && game) {
            printf("going to %d \n", current_move.m);
            if (current_player == player1_handle)
                position_player_handle1[++i1] = current_move;
            else
                position_player_handle2[++i2] = current_move;
        } else if (current_move.t == WALL) {
            printf("put a wall on : %d -> %d and %d -> %d\n", current_move.e[0].fr,
                   current_move.e[0].to, current_move.e[1].fr, current_move.e[1].to);
            // graph_remove_edge(graph, current_move.e[0]);
            // graph_remove_edge(graph, current_move.e[1]); done in is_possible_move
        }
        if ((current_move.t == MOVE) && (on_obj(graph, current_move) != -1)) {
            if (current_player == player1_handle) {
                spot[player1_id][on_obj(graph, current_move)] = 1;
            } else {
                spot[player2_id][on_obj(graph, current_move)] = 1;
            }
        }

        int current_player_id = (current_player == player1_handle) ? player1_id : player2_id;
        struct move_t *current_positions =
            (current_player == player1_handle) ? position_player_handle1 : position_player_handle2;
        int current_i = (current_player == player1_handle) ? i1 : i2;
        if (is_game_over(spot, current_positions[current_i], current_positions[0],
                         current_player_id, num_objectives) == 1) {
            printf("\nPLAYER %d HAS WON !\n", current_player_id);
            game = 0;
        }

        current_player = switch_player(current_player, player1_handle, player2_handle);
        i++;

        if (i % 2 == 0 && game && i < max_round) {
            printf("NEXT ROUND\n");
        }
    }

    if (i == max_round) {
        printf("\nNobody win %d tour done\n", max_round / 2);
    }

    printf("\n=================END OF GAME=================\n");

    for (int i = 0; i < 2; i++)
        free(spot[i]);
    free(spot);

    free(position_player_handle1);
    free(position_player_handle2);
    //  Finalize players
    finalize_player(player1_handle);
    finalize_player(player2_handle);

    // Clean up
    graph_destroy(graph);
    dlclose(player1_handle);
    dlclose(player2_handle);

    return 0;
}
