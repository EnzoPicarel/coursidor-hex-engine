#include "../src/gen_graph.h"
#include <stdio.h>

void export_graph_to_dot(struct graph_t *graph, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f)
        return;

    // Header avec paramètres optimisés
    fprintf(f, "digraph G {\n");
    fprintf(f, "  node [shape=circle, style=filled, fillcolor=lightgrey, "
               "fixedsize=true, width=0.1, height=0.1, fontsize=6];\n");
    fprintf(f, "  edge [len=1.2, arrowsize=0.2];\n");
    fprintf(f, "  graph [splines=true, overlap=false];\n");

    // Positions hexagonales optimisées
    // Ligne du haut (0, 1)
    fprintf(f, "  0 [pos=\"2.0,4.0!\", label=\"0\"];\n");
    fprintf(f, "  1 [pos=\"4.0,4.0!\", label=\"1\"];\n");

    // Ligne du milieu (2, 3, 4) - plus espacée
    fprintf(f, "  2 [pos=\"1.0,2.0!\", label=\"2\"];\n");
    fprintf(f, "  3 [pos=\"3.0,2.0!\", label=\"3\"];\n");
    fprintf(f, "  4 [pos=\"5.0,2.0!\", label=\"4\"];\n");

    // Ligne du bas (5, 6)
    fprintf(f, "  5 [pos=\"2.0,0.0!\", label=\"5\"];\n");
    fprintf(f, "  6 [pos=\"4.0,0.0!\", label=\"6\"];\n");

    // Arêtes minimalistes
    for (size_t i = 0; i < graph->num_vertices; i++) {
        for (size_t j = 0; j < graph->num_vertices; j++) {
            unsigned int val = gsl_spmatrix_uint_get(graph->t, i, j);
            if (val > 0) {
                fprintf(f, "  %zu -> %zu [dir=both];\n", i, j);
            }
        }
    }
    fprintf(f, "}\n");
    fclose(f);
}

// Création d'un graphe correspondant exactement à l'exemple du sujet
// t = [[ 0 3 4 0 ]
//      [ 6 0 0 4 ]
//      [ 1 0 0 3 ]
//      [ 0 1 6 0 ]]

struct graph_t *create_example_graph() {
    struct graph_t *graph = graph_create(4);
    struct edge_t edge;

    edge = (struct edge_t){0, 1};
    graph_add_edge(graph, edge, E); // t[0][1] = 3 (E)

    edge = (struct edge_t){0, 2};
    graph_add_edge(graph, edge, SE); // t[0][2] = 4 (SE)

    edge = (struct edge_t){1, 3};
    graph_add_edge(graph, edge, SE); // t[1][3] = 4 (SE)

    edge = (struct edge_t){2, 3};
    graph_add_edge(graph, edge, E); // t[2][3] = 3 (E)

    graph_ensure_csr_format(graph);

    return graph;
}

void create_example_graph_test() {
    // Test avec le graphe de l'exemple
    printf("=== Test du graphe de l'exemple ===\n");
    struct graph_t *example = create_example_graph();
    print_graph_info(example);

    printf("\n=== Matrice d'adjacence du graphe de l'exemple ===\n");
    print_adjacency_matrix(example);

    graph_destroy(example);
}

void test_get_neighbors() {
    printf("=== test %s ===\n", __func__);
    struct graph_t *graph = graph_create_triangular(3);

    unsigned int c = 0;
    unsigned int *array = graph_get_neighbors(graph, 0, &c);
    printf("neighbors of 0 : \n");
    for (unsigned int i = 0; i < c; i++)
        printf("%d, ", array[i]);

    unsigned int *array1 = graph_get_neighbors(graph, 9, &c);
    printf("\nneighbors of vertex in center : \n");
    for (unsigned int i = 0; i < c; i++)
        printf("%d, ", array1[i]);

    free(array1);
    free(array);
    graph_destroy(graph);
    printf("DONE\n");
}

void create_graph_test() {
    // Test de création d'un graphe triangular
    printf("\n=== Test du graphe triangular ===\n");
    struct graph_t *triangular = graph_create_triangular(3);
    print_graph_info(triangular);

    // Afficher la matrice d'adjacence complète
    printf("\n=== Matrice d'adjacence du graphe triangular ===\n");
    print_adjacency_matrix(triangular);

    export_graph_to_dot(triangular, "graph.dot");
    system("neato -n -Tpng -Gdpi=500 graph.dot -o graph.png");
    graph_destroy(triangular);

    // Test de création d'un graphe cyclique
    printf("\n=== Test du graphe cyclique ===\n");
    struct graph_t *cyclic = graph_create_cyclic(3);
    print_graph_info(cyclic);

    // Afficher la matrice d'adjacence complète
    printf("\n=== Matrice d'adjacence du graphe cyclique ===\n");
    print_adjacency_matrix(cyclic);

    graph_destroy(cyclic);
}