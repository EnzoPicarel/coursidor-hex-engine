#include <stdio.h>

// Declare the test functions from test_graph.c
void create_example_graph_test();
void create_graph_test();
void test_get_neighbors();

// Declare the test functions from test_player.c
void get_player_name_test();
void initialize_test();
void play_test();
void finalize_test();
void cmp_length_test();
void extract_min_test();
void relax_test();
void is_in_direction_test();
void get_direction_test();
void a_star_test();
void max_length_move_test();
void outgoing_edge_list_test();
void is_empty_color_test();
void hexagonal_distance_test();
void build_index_to_hex_table_test();
void get_close_neighbor_test();
void get_edge_neighbors_test();

int main()
{
    // Run graph tests
    printf("=== Graph Tests ===\n");
    create_example_graph_test();
    create_graph_test();
    test_get_neighbors();

    // Run player tests
    printf("\n=== Player Tests ===\n");
    build_index_to_hex_table_test();
    hexagonal_distance_test();
    get_player_name_test();
    // initialize_test();
    play_test();
    // finalize_test();
    cmp_length_test();
    extract_min_test();
    relax_test();
    is_in_direction_test();
    get_direction_test();
    a_star_test();
    max_length_move_test();
    is_empty_color_test();
    outgoing_edge_list_test();
    get_close_neighbor_test();
    get_edge_neighbors_test();
    printf("\nAll tests completed successfully.\n");
    return 0;
}
