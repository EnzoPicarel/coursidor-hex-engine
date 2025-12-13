#include "../src/astar_player.h"
#include "../src/gen_graph.h"
#include "../src/graph.h"
#include "../src/player.h"
#include <assert.h>

#ifndef INF
#define INF 1000000
#endif

void build_index_to_hex_table_test()
{
    printf("Test %s ", __func__);

    struct graph_t *g = graph_create(7);

    struct edge_t edge;

    for (unsigned int dir = FIRST_DIR; dir < NUM_DIRS + 1; dir++)
    {
        edge = (struct edge_t){0, dir};
        graph_add_edge(g, edge, dir);
        edge = (struct edge_t){dir, (dir + 1) % NUM_DIRS};
        graph_add_edge(g, edge, (dir + 2) % NUM_DIRS);
    }

    graph_ensure_csr_format(g);
    initialize(0, g);

    struct hex_pos *table = malloc(sizeof(struct hex_pos) * 7);
    build_index_to_hex_table(table, 2);
    assert(table[0].q == 0 && table[0].r == -1);
    assert(table[1].q == 1 && table[1].r == -1);
    assert(table[2].q == -1 && table[2].r == 0);
    assert(table[3].q == 0 && table[3].r == 0);
    assert(table[4].q == 1 && table[4].r == 0);
    assert(table[5].q == -1 && table[5].r == 1);
    assert(table[6].q == 0 && table[6].r == 1);
    finalize();
    free(table);

    printf("OK\n");
}

void hexagonal_distance_test()
{
    printf("Test %s ", __func__);

    struct graph_t *g = graph_create(7);

    struct edge_t edge;

    edge = (struct edge_t){3, 0};
    graph_add_edge(g, edge, NW);

    edge = (struct edge_t){3, 1};
    graph_add_edge(g, edge, NE);

    edge = (struct edge_t){3, 2};
    graph_add_edge(g, edge, W);

    edge = (struct edge_t){3, 4};
    graph_add_edge(g, edge, E);

    edge = (struct edge_t){3, 5};
    graph_add_edge(g, edge, SW);

    edge = (struct edge_t){3, 6};
    graph_add_edge(g, edge, SE);

    edge = (struct edge_t){0, 1};
    graph_add_edge(g, edge, E);

    edge = (struct edge_t){1, 4};
    graph_add_edge(g, edge, SE);

    edge = (struct edge_t){4, 6};
    graph_add_edge(g, edge, SW);

    edge = (struct edge_t){6, 5};
    graph_add_edge(g, edge, W);

    edge = (struct edge_t){5, 2};
    graph_add_edge(g, edge, NW);

    edge = (struct edge_t){2, 0};
    graph_add_edge(g, edge, NE);

    graph_ensure_csr_format(g);
    initialize(0, g);

    struct hex_pos *table = malloc(sizeof(struct hex_pos) * 7);
    build_index_to_hex_table(table, 2);

    assert(hexagonal_distance(0, 3, table) == 1);
    assert(hexagonal_distance(1, 3, table) == 1);
    assert(hexagonal_distance(2, 3, table) == 1);
    assert(hexagonal_distance(4, 3, table) == 1);
    assert(hexagonal_distance(6, 3, table) == 1);
    assert(hexagonal_distance(5, 3, table) == 1);

    assert(hexagonal_distance(0, 1, table) == 1);

    assert(hexagonal_distance(0, 6, table) == 2);

    assert(hexagonal_distance(0, 4, table) == 2);

    finalize();
    free(table);
    printf("OK\n");
}

void get_player_name_test()
{
    printf("Test %s ", __func__);

    const char *c = get_player_name();
    assert(c != NULL);

    printf("OK\n");
};

/*
void initialize_test() {

};
*/

void play_test()
{
    printf("Test %s ", __func__);

    // A first test without an opposing player
    struct graph_t *g = graph_create_triangular(4);
    unsigned int tab_obj[4] = {3, 33, 24, 12};
    graph_add_objectives(g, 4, tab_obj);
    g->start[0] = 15;
    g->start[1] = 21;

    graph_ensure_csr_format(g);
    initialize(0, g);

    const struct move_t previous_move = {.c = 1, .t = 0, .m = 0, .e = {{0, 0}, {0, 0}}};
    int is_expected_move(const struct move_t expected_move, struct move_t move)
    {
        return (move.c == expected_move.c && move.t == expected_move.t &&
                move.m == expected_move.m && move.e[0].to == expected_move.e[0].to &&
                move.e[0].fr == expected_move.e[0].fr && move.e[1].to == expected_move.e[1].to &&
                move.e[1].fr == expected_move.e[1].fr);
    }

    const struct move_t expected_move1 = {.c = 0, .t = 2, .m = 22, .e = {{0, 0}, {0, 0}}};
    if (is_expected_move(expected_move1, previous_move))
    {
        printf("test to compile without warning\n");
    }
    struct move_t move_test = play(previous_move);

    assert(is_expected_move(expected_move1, move_test) != 0);

    const struct move_t expected_move2 = {.c = 0, .t = 2, .m = 33, .e = {{0, 0}, {0, 0}}};
    assert(is_expected_move(expected_move2, play(previous_move)) != 0);

    const struct move_t expected_move4 = {.c = 0, .t = 2, .m = 29, .e = {{0, 0}, {0, 0}}};
    assert(is_expected_move(expected_move4, play(previous_move)) != 0);

    const struct move_t expected_move5 = {.c = 0, .t = 2, .m = 24, .e = {{0, 0}, {0, 0}}};
    assert(is_expected_move(expected_move5, play(previous_move)) != 0);

    const struct move_t expected_move6 = {.c = 0, .t = 2, .m = 12, .e = {{0, 0}, {0, 0}}};
    assert(is_expected_move(expected_move6, play(previous_move)) != 0);

    const struct move_t expected_move7 = {.c = 0, .t = 2, .m = 3, .e = {{0, 0}, {0, 0}}};
    assert(is_expected_move(expected_move7, play(previous_move)) != 0);

    struct move_t move = play(previous_move);
    assert(move.t == 2);
    assert(move.m == 2 || move.m == 7);
    if (move.m == 2)
    {
        move = play(previous_move);
        assert(move.t == 2);
        assert(move.m == 0 || move.m == 11 || move.m == 6);
        if (move.m == 0)
        {
            move = play(previous_move);
            assert(move.t == 2);
            assert(move.m == 9 || move.m == 4);

            move = play(previous_move);
            assert(move.t == 2);
            assert(move.m == 15);
        }
        else if (move.m == 11 || move.m == 6)
        {
            move = play(previous_move);
            assert(move.t == 2);
            assert(move.m == 17);

            move = play(previous_move);
            assert(move.t == 2);
            assert(move.m == 15);
        }
    }
    else if (move.m == 7)
    {
        move = play(previous_move);
        assert(move.t == 2);
        assert(move.m == 18 || move.m == 6 || move.m == 5);

        if (move.m == 18)
        {
            move = play(previous_move);
            assert(move.t == 2);
            assert(move.m == 16);

            move = play(previous_move);
            assert(move.t == 2);
            assert(move.m == 15);
        }
        else if (move.m == 7)
        {
            move = play(previous_move);
            assert(move.t == 2);
            assert(move.m == 17);

            move = play(previous_move);
            assert(move.t == 2);
            assert(move.m == 15);
        }
        else if (move.m == 6)
        {
            move = play(previous_move);
            assert(move.t == 2);
            assert(move.m == 16 || move.m == 4);

            move = play(previous_move);
            assert(move.t == 2);
            assert(move.m == 15);
        }
    }
    finalize();
    printf("OK\n");
}

void held_karp_test()
{
    printf("Test %s", __func__);
}

/*
void finalize_test() {

};
*/

void cmp_length_test()
{
    printf("Test %s ", __func__);
    unsigned int a = 3;
    unsigned int b = 4;
    unsigned int c = INF; // inf
    assert(cmp_length(a, b) != 0);
    assert(cmp_length(a, a) == 0);
    assert(cmp_length(b, a) == 0);
    assert(cmp_length(a, c) != 0);
    assert(cmp_length(c, a) == 0);
    printf("OK\n");
};

void extract_min_test()
{
    printf("Test %s ", __func__);
    unsigned int *visit_state = malloc(10 * sizeof(unsigned int));
    unsigned int *d = malloc(10 * sizeof(unsigned int));
    unsigned int *h = malloc(10 * sizeof(unsigned int));
    int n = 10;

    for (int i = 0; i < n; i++)
    {
        visit_state[i] = 1;
        h[i] = 0;
    }
    d[0] = 6;
    d[1] = 5;
    d[2] = 3;
    d[3] = 1;
    d[4] = 2;
    d[5] = 2;
    d[6] = 12;
    d[7] = 7;
    d[8] = INF; // inf
    d[9] = 5;

    unsigned int r;

    r = extract_min(visit_state, d, h, n);
    visit_state[r] = 2;
    assert(r == 3 && visit_state[3] == 2);
    r = extract_min(visit_state, d, h, n);
    visit_state[r] = 2;
    assert(r == 4 && visit_state[4] == 2);
    r = extract_min(visit_state, d, h, n);
    visit_state[r] = 2;
    assert(r == 5 && visit_state[5] == 2);
    r = extract_min(visit_state, d, h, n);
    visit_state[r] = 2;
    assert(r == 2 && visit_state[2] == 2);
    r = extract_min(visit_state, d, h, n);
    visit_state[r] = 2;
    assert(r == 1 && visit_state[1] == 2);
    r = extract_min(visit_state, d, h, n);
    visit_state[r] = 2;
    assert(r == 9 && visit_state[9] == 2);
    r = extract_min(visit_state, d, h, n);
    visit_state[r] = 2;
    assert(r == 0 && visit_state[0] == 2);
    r = extract_min(visit_state, d, h, n);
    visit_state[r] = 2;
    assert(r == 7 && visit_state[7] == 2);
    r = extract_min(visit_state, d, h, n);
    visit_state[r] = 2;
    assert(r == 6 && visit_state[6] == 2);
    r = extract_min(visit_state, d, h, n);
    visit_state[r] = 2;
    assert(r == 8 && visit_state[8] == 2);
    free(visit_state);
    free(d);
    free(h);
    printf("OK\n");
};

void relax_test()
{
    printf("Test %s ", __func__);
    unsigned int *d = malloc(5 * sizeof(unsigned int));
    unsigned int *parent = malloc(5 * sizeof(unsigned int));
    unsigned int *visit_state = malloc(5 * sizeof(unsigned int));

    d[0] = 1;
    d[1] = 2;
    d[2] = 1;
    d[3] = 4;
    d[4] = 1;

    parent[0] = 0;
    parent[1] = 2;
    parent[2] = 3;
    parent[3] = 1;
    parent[4] = 3;

    visit_state[0] = 2;
    visit_state[1] = 2;
    visit_state[2] = 0;
    visit_state[3] = 0;
    visit_state[4] = 1;

    relax(4, 3, 2, d, parent, visit_state);

    assert(visit_state[3] = 1);

    assert(parent[3] = 4);

    assert(d[4] = 1);
    assert(d[3] = 3);

    free(d);
    free(parent);
    free(visit_state);
    printf("OK\n");
};

void is_in_direction_test()
{
    printf("Test %s ", __func__);

    struct graph_t *g = graph_create(9);
    struct edge_t edge;

    for (int i = 1; i < 7; i++)
    {
        edge = (struct edge_t){0, i};
        graph_add_edge(g, edge, (unsigned int)i);
    }

    edge = (struct edge_t){1, 7};
    graph_add_edge(g, edge, 1);

    edge = (struct edge_t){7, 8};
    graph_add_edge(g, edge, 1);

    graph_ensure_csr_format(g);

    initialize(0, g);

    for (unsigned int d = 1; d < 7; d++)
    {
        assert(is_in_direction(0, d, d, 3) != 0);
        assert(is_in_direction(0, d, (d + 1) % NUM_DIRS, 3) == 0);
    }

    assert(is_in_direction(0, 7, 1, 3) != 0);
    assert(is_in_direction(0, 7, 2, 3) == 0);

    assert(is_in_direction(0, 8, 1, 3) != 0);
    assert(is_in_direction(0, 8, 2, 3) == 0);

    finalize();
    printf("OK\n");
}

void get_direction_test()
{
    printf("Test %s ", __func__);

    struct graph_t *g = graph_create(9);
    struct edge_t edge;

    for (int i = 1; i < 7; i++)
    {
        edge = (struct edge_t){0, i};
        graph_add_edge(g, edge, (unsigned int)i);
    }

    edge = (struct edge_t){1, 7};
    graph_add_edge(g, edge, 1);

    edge = (struct edge_t){7, 8};
    graph_add_edge(g, edge, 1);

    graph_ensure_csr_format(g);

    initialize(0, g);

    for (unsigned int d = 1; d < 7; d++)
    {
        assert(get_direction(0, d) == d);
    }

    assert(get_direction(0, 7) == 1);

    assert(get_direction(0, 8) == 1);

    finalize();

    printf("OK\n");
}

void a_star_test()
{
    printf("Test %s ", __func__);

    struct graph_t *g = graph_create(10);
    g->start[0] = 0;
    g->start[1] = 5;
    struct edge_t edge;

    edge = (struct edge_t){0, 1};
    graph_add_edge(g, edge, 6);

    edge = (struct edge_t){0, 2};
    graph_add_edge(g, edge, 1);

    edge = (struct edge_t){0, 3};
    graph_add_edge(g, edge, 2);

    edge = (struct edge_t){0, 4};
    graph_add_edge(g, edge, 3);

    edge = (struct edge_t){0, 5};
    graph_add_edge(g, edge, 4);

    edge = (struct edge_t){0, 6};
    graph_add_edge(g, edge, 5);

    edge = (struct edge_t){1, 2};
    graph_add_edge(g, edge, 2);

    edge = (struct edge_t){1, 6};
    graph_add_edge(g, edge, 4);

    edge = (struct edge_t){2, 3};
    graph_add_edge(g, edge, 3);

    edge = (struct edge_t){3, 4};
    graph_add_edge(g, edge, 4);

    edge = (struct edge_t){4, 5};
    graph_add_edge(g, edge, 5);

    edge = (struct edge_t){4, 7};
    graph_add_edge(g, edge, 3);

    edge = (struct edge_t){5, 6};
    graph_add_edge(g, edge, 6);

    edge = (struct edge_t){6, 9};
    graph_add_edge(g, edge, 5);

    edge = (struct edge_t){7, 8};
    graph_add_edge(g, edge, 3);

    /*
    Graph layout:

        2          3

    1       0      4      7       8

         6          5

      9

    */

    graph_ensure_csr_format(g);
    initialize(0, g);

    unsigned int *p1 = a_star(0, 8, 0); // no previous direction specified
    // expected path: 0, 4, 8

    printf("verif : %u %u %u %u\n", p1[0], p1[1], p1[2], p1[3]);

    assert(p1[0] == 3);
    assert(p1[1] == 8);
    assert(p1[2] == 4);
    assert(p1[3] == 0);

    free(p1);

    unsigned int *p2 = a_star(0, 8, 3); // coming from 1, direction taken: EAST = 3
    // expected path: 0, 8

    printf("verif : %u %u %u %u\n", p2[0], p2[1], p2[2], p2[3]);
    assert(p2[0] == 2);
    assert(p2[1] == 8);
    assert(p2[2] == 0);

    free(p2);

    unsigned int *p3 = a_star(0, 9, 6); // coming from 4, direction taken: WEST = 6
    // expected path: 0, 9
    assert(p3[0] == 2);
    assert(p3[1] == 9);
    assert(p3[2] == 0);

    free(p3);

    unsigned int *p4 = a_star(2, 9, 0); // no previous direction specified
    // expected path: 2, 0, 9

    assert(p4[0] == 3);
    assert(p4[1] == 9);
    assert(p4[2] == 0);
    assert(p4[3] == 2);

    free(p4);

    unsigned int *p5 = a_star(1, 9, 6); // coming from 0, direction taken: WEST = 6
    // expected path: 1, 0, 6, 9

    assert(p5[0] == 3);
    assert(p5[1] == 9);
    assert(p5[2] == 6);
    assert(p5[3] == 1);

    free(p5);

    finalize();
    printf("OK\n");
};

void max_length_move_test()
{
    printf("Test %s ", __func__);
    enum dir_t d1 = 1;
    enum dir_t d2 = 2;
    enum dir_t d3 = 3;
    enum dir_t d4 = 4;
    enum dir_t d5 = 5;
    enum dir_t d6 = 6;
    assert(max_length_move(d1, d1) == 3);
    assert(max_length_move(d1, d2) == 2);
    assert(max_length_move(d1, d3) == 1);
    assert(max_length_move(d1, d4) == 1);
    assert(max_length_move(d1, d5) == 1);
    assert(max_length_move(d1, d6) == 2);

    assert(max_length_move(d2, d1) == 2);
    assert(max_length_move(d2, d2) == 3);
    assert(max_length_move(d2, d3) == 2);
    assert(max_length_move(d2, d4) == 1);
    assert(max_length_move(d2, d5) == 1);
    assert(max_length_move(d2, d6) == 1);

    assert(max_length_move(d3, d1) == 1);
    assert(max_length_move(d3, d2) == 2);
    assert(max_length_move(d3, d3) == 3);
    assert(max_length_move(d3, d4) == 2);
    assert(max_length_move(d3, d5) == 1);
    assert(max_length_move(d3, d6) == 1);

    assert(max_length_move(d4, d1) == 1);
    assert(max_length_move(d4, d2) == 1);
    assert(max_length_move(d4, d3) == 2);
    assert(max_length_move(d4, d4) == 3);
    assert(max_length_move(d4, d5) == 2);
    assert(max_length_move(d4, d6) == 1);

    assert(max_length_move(d5, d1) == 1);
    assert(max_length_move(d5, d2) == 1);
    assert(max_length_move(d5, d3) == 1);
    assert(max_length_move(d5, d4) == 2);
    assert(max_length_move(d5, d5) == 3);
    assert(max_length_move(d5, d6) == 2);

    assert(max_length_move(d6, d1) == 2);
    assert(max_length_move(d6, d2) == 1);
    assert(max_length_move(d6, d3) == 1);
    assert(max_length_move(d6, d4) == 1);
    assert(max_length_move(d6, d5) == 2);
    assert(max_length_move(d6, d6) == 3);
    printf("OK\n");
};

void is_empty_color_test()
{
    printf("Test %s ", __func__);
    unsigned int *visit_state = malloc(3 * sizeof(unsigned int));
    visit_state[0] = 1;
    visit_state[1] = 0;
    visit_state[2] = 2;
    assert(is_empty_color(visit_state, 0, 3) == 0);
    assert(is_empty_color(visit_state, 1, 3) == 0);
    assert(is_empty_color(visit_state, 2, 3) == 0);

    visit_state[0] = 0;

    assert(is_empty_color(visit_state, 1, 3) != 0);

    visit_state[2] = 0;

    assert(is_empty_color(visit_state, 2, 3) != 0);

    free(visit_state);
    printf("OK\n");
}

void outgoing_edge_list_test()
{
    printf("Test %s ", __func__);

    struct graph_t *g = graph_create_triangular(3);
    g->start[0] = 2;
    g->start[1] = 2;
    initialize(0, g);

    void reset(vertex_t * t, unsigned int *d)
    {
        for (int i = 0; i < 10; i++)
        {
            t[i] = 19; // > nb sommets
            d[i] = 0;  // < toute dist
        }
    }
    /*
    void printest(vertex_t *test_tab) {
        printf("outgoing : ");
        for (int i=0;i<10;i++) {
            printf("%u, ", test_tab[i]);
        }
        printf("\n");
    }*/

    vertex_t *test_tab = malloc(10 * sizeof(vertex_t));
    unsigned int *test_dist = malloc(10 * sizeof(unsigned int));

    unsigned int v = 4;
    enum dir_t before = SE;
    reset(test_tab, test_dist);
    outgoing_edge_list(before, v, test_tab, test_dist);
    // printest(test_tab);
    //  Un premier test simple
    assert(test_tab[0] == 0 && test_dist[0] == 1);
    assert(test_tab[1] == 1 && test_dist[1] == 1);
    assert(test_tab[2] == 3 && test_dist[2] == 1);
    assert(test_tab[3] == 5 && test_dist[3] == 1);
    assert(test_tab[4] == 6 && test_dist[4] == 2);
    assert(test_tab[5] == 9 && test_dist[5] == 1);
    assert(test_tab[6] == 14 && test_dist[6] == 2);
    assert(test_tab[7] == 18 && test_dist[7] == 3);
    assert(test_tab[8] == 8 && test_dist[8] == 1);
    assert(test_tab[9] == 12 && test_dist[9] == 2);
    reset(test_tab, test_dist);

    // En mettant un obstacle lointain
    struct edge_t e1 = {9, 14};
    struct edge_t e2 = {9, 10};
    graph_remove_edge(g, e1);
    graph_remove_edge(g, e2);

    outgoing_edge_list(before, v, test_tab, test_dist);
    // printest(test_tab);
    assert(test_tab[0] == 0 && test_dist[0] == 1);
    assert(test_tab[1] == 1 && test_dist[1] == 1);
    assert(test_tab[2] == 3 && test_dist[2] == 1);
    assert(test_tab[3] == 5 && test_dist[3] == 1);
    assert(test_tab[4] == 6 && test_dist[4] == 2);
    assert(test_tab[5] == 9 && test_dist[5] == 1);
    assert(test_tab[6] == 8 && test_dist[6] == 1);
    assert(test_tab[7] == 12 && test_dist[7] == 2);
    assert(test_tab[8] == 10000000 && test_dist[8] == 0);
    assert(test_tab[9] == 19 && test_dist[9] == 0);
    reset(test_tab, test_dist);

    // En mettant un obstacle proche
    e1.fr = 4;
    e1.to = 5;
    e2.fr = 4;
    e2.to = 9;
    graph_remove_edge(g, e1);
    graph_remove_edge(g, e2);

    outgoing_edge_list(before, v, test_tab, test_dist);
    // printest(test_tab);
    assert(test_tab[0] == 0 && test_dist[0] == 1);
    assert(test_tab[1] == 1 && test_dist[1] == 1);
    assert(test_tab[2] == 3 && test_dist[2] == 1);
    assert(test_tab[3] == 8 && test_dist[3] == 1);
    assert(test_tab[4] == 12 && test_dist[4] == 2);
    assert(test_tab[5] == 10000000 && test_dist[5] == 0);
    assert(test_tab[6] == 19 && test_dist[6] == 0);
    assert(test_tab[7] == 19 && test_dist[7] == 0);
    assert(test_tab[8] == 19 && test_dist[8] == 0);
    assert(test_tab[9] == 19 && test_dist[9] == 0);
    reset(test_tab, test_dist);

    // Sur une bordure
    v = 18;

    outgoing_edge_list(before, v, test_tab, test_dist);
    // printest(test_tab);
    assert(test_tab[0] == 14 && test_dist[2] == 1);
    assert(test_tab[1] == 15 && test_dist[1] == 1);
    assert(test_tab[2] == 17 && test_dist[0] == 1);
    assert(test_tab[3] == 10000000 && test_dist[3] == 0);
    assert(test_tab[4] == 19 && test_dist[4] == 0);
    assert(test_tab[5] == 19 && test_dist[5] == 0);
    assert(test_tab[6] == 19 && test_dist[6] == 0);
    assert(test_tab[7] == 19 && test_dist[7] == 0);
    assert(test_tab[8] == 19 && test_dist[8] == 0);
    assert(test_tab[9] == 19 && test_dist[9] == 0);
    reset(test_tab, test_dist);

    free(test_dist);
    free(test_tab);
    finalize();
    printf("OK\n");
};

void get_close_neighbor_test()
{
    printf("Test %s ", __func__);

    struct graph_t *g = graph_create_triangular(3);
    initialize(0, g);

    // Un premier test simple
    unsigned int start = 4;
    unsigned int dest = 9;
    assert(get_close_neighbor(start, dest) == 9);

    dest = 14;
    assert(get_close_neighbor(start, dest) == 9);

    dest = 18;
    assert(get_close_neighbor(start, dest) == 9);

    // Proche d'un mur
    struct edge_t e1 = {9, 10};
    struct edge_t e2 = {9, 14};
    graph_remove_edge(g, e1);
    graph_remove_edge(g, e2);

    dest = 14;
    assert(get_close_neighbor(start, dest) == 9);

    dest = 18;
    assert(get_close_neighbor(start, dest) == 9);

    finalize();
    printf(" OK\n");
}

void get_edge_neighbors_test()
{
    printf("Test %s ", __func__);

    struct graph_t *g = graph_create_triangular(3);
    initialize(0, g);

    struct edge_t *test_neighbors = malloc(4 * sizeof(struct edge_t));
    struct edge_t e = {4, 9};
    struct edge_t e_i = {19, 19};
    void init_edge(struct edge_t * n)
    {
        for (int i = 0; i < 4; i++)
        {
            n[i] = e_i;
        }
    }
    init_edge(test_neighbors);
    get_edge_neighbors(test_neighbors, e);

    struct edge_t current_edge_check = test_neighbors[0];
    unsigned int e1 = 5;
    unsigned int e2 = 8;
    assert((int)current_edge_check.fr != 19 && (int)current_edge_check.to != 19);
    assert(current_edge_check.fr == e.fr &&
           (current_edge_check.to == e1 || current_edge_check.to == e2));

    if (current_edge_check.to == e1)
    {
        current_edge_check = test_neighbors[2];
        assert((int)current_edge_check.fr != 19 && (int)current_edge_check.to != 19);
        assert(current_edge_check.fr == e.fr && current_edge_check.to == e2);
    }
    else if (current_edge_check.to == e2)
    {
        current_edge_check = test_neighbors[2];
        assert((int)current_edge_check.fr != 19 && (int)current_edge_check.to != 19);
        assert(current_edge_check.fr == e.fr && current_edge_check.to == e1);
    }

    current_edge_check = test_neighbors[1];
    assert((int)current_edge_check.fr != 19 && (int)current_edge_check.to != 19);
    assert(current_edge_check.fr == e.to &&
           (current_edge_check.to == e1 || current_edge_check.to == e2));

    if (current_edge_check.to == e1)
    {
        current_edge_check = test_neighbors[3];
        assert((int)current_edge_check.fr != 19 && (int)current_edge_check.to != 19);
        assert(current_edge_check.fr == e.to && current_edge_check.to == e2);
    }
    else if (current_edge_check.to == e2)
    {
        current_edge_check = test_neighbors[3];
        assert((int)current_edge_check.fr != 19 && (int)current_edge_check.to != 19);
        assert(current_edge_check.fr == e.to && current_edge_check.to == e1);
    }

    free(test_neighbors);
    finalize();
    printf("OK\n");
}
