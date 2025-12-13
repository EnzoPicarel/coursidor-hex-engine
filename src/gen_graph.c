#include "gen_graph.h"
#include <string.h>

// Create an empty graph
struct graph_t *graph_create(unsigned int num_vertices)
{
    struct graph_t *graph = malloc(sizeof(struct graph_t));
    if (!graph)
        return NULL;

    graph->t = gsl_spmatrix_uint_alloc(num_vertices, num_vertices);
    if (!graph->t)
    {
        free(graph);
        return NULL;
    }

    graph->num_vertices = num_vertices;
    graph->num_edges = 0;
    graph->objectives = NULL;
    graph->num_objectives = 0;

    vertex_t start[NUM_PLAYERS] = {0, num_vertices - 1};
    graph->start[0] = start[0];
    graph->start[1] = start[1];

    return graph;
}

// Free all resources held by a graph
void graph_destroy(struct graph_t *graph)
{
    if (graph)
    {
        if (graph->t)
        {
            gsl_spmatrix_uint_free(graph->t);
        }
        if (graph->objectives)
        {
            free(graph->objectives);
        }
        free(graph);
    }
}

// Add an edge to the graph
bool graph_add_edge(struct graph_t *graph, struct edge_t edge, enum dir_t dir)
{
    if (!graph || edge.fr >= graph->num_vertices || edge.to >= graph->num_vertices ||
        dir == NO_EDGE)
    {
        return false;
    }

    // Check if the edge already exists
    if (gsl_spmatrix_uint_get(graph->t, edge.fr, edge.to) != NO_EDGE &&
        gsl_spmatrix_uint_get(graph->t, edge.fr, edge.to) != WALL_DIR)
    {
        return false;
    }

    // Add the edge in both directions (undirected graph)
    gsl_spmatrix_uint_set(graph->t, edge.fr, edge.to, dir);
    gsl_spmatrix_uint_set(graph->t, edge.to, edge.fr, opposite_dir(dir));

    graph->num_edges++;
    return true;
}

bool graph_add_edge_compressed(struct graph_t *graph, struct edge_t edge, enum dir_t dir)
{
    manual_edge_set(graph, edge, dir);

    graph->num_edges++;
    return true;
}

// Remove an edge from the graph
bool graph_remove_edge(struct graph_t *graph, struct edge_t edge)
{
    if (!graph || edge.fr >= graph->num_vertices || edge.to >= graph->num_vertices)
    {
        return false;
    }

    // Check if the edge exists
    if (gsl_spmatrix_uint_get(graph->t, edge.fr, edge.to) == NO_EDGE)
    {
        return false;
    }

    // Remove the edge in both directions
    // gsl_spmatrix_uint_set(graph->t, edge.fr, edge.to, WALL_DIR);
    // gsl_spmatrix_uint_set(graph->t, edge.to, edge.fr, WALL_DIR);
    manual_edge_set(graph, edge, WALL_DIR);

    graph->num_edges--;
    return true;
}

void manual_edge_set(struct graph_t *graph, struct edge_t edge, enum dir_t replace)
{
    unsigned int from = edge.fr;
    unsigned int to = edge.to;

    gsl_spmatrix_uint *mat = graph->t;
    int *p = mat->p;
    int *i = mat->i;
    unsigned int *data = mat->data;

    for (int j = p[from]; j < p[from + 1]; ++j)
    {
        if (i[j] == (int)to)
        {
            data[j] = replace;
            break;
        }
    }

    for (int j = p[to]; j < p[to + 1]; ++j)
    {
        if (i[j] == (int)from)
        {
            if (replace != WALL_DIR && replace != NO_EDGE)
            {
                data[j] = opposite_dir(replace);
            }
            else
            {
                data[j] = replace;
            }
            break;
        }
    }
}

// Get the direction of an edge
enum dir_t graph_get_edge_direction(struct graph_t *graph, struct edge_t edge)
{
    if (!graph || edge.fr >= graph->num_vertices || edge.to >= graph->num_vertices)
    {
        return NO_EDGE;
    }
    return gsl_spmatrix_uint_get(graph->t, edge.fr, edge.to);
}

// Check if an edge exists
bool graph_has_edge(struct graph_t *graph, struct edge_t edge)
{
    if (!graph || edge.fr >= graph->num_vertices || edge.to >= graph->num_vertices)
    {
        return false;
    }
    return gsl_spmatrix_uint_get(graph->t, edge.fr, edge.to) != NO_EDGE;
}

// Add objectives to the graph
bool graph_add_objectives(struct graph_t *graph, unsigned int max_objectives, unsigned int *tab)
{
    if (!graph)
    {
        printf("ERROR\n");
        return false;
    }

    graph->num_objectives = 0;
    vertex_t *objectives = malloc(max_objectives * sizeof(vertex_t));

    if (!objectives)
    {
        return false;
    }

    graph->objectives = objectives;
    // Adjust based on chosen bases
    for (unsigned int i = 0; i < max_objectives; i++)
    {
        graph->objectives[i] = tab[i];
        graph->num_objectives++;
    }
    return true;
}

// Ensure the matrix uses CSR storage
void graph_ensure_csr_format(struct graph_t *graph)
{
    if (strcmp(gsl_spmatrix_uint_type(graph->t), "CSR") != 0)
    {
        gsl_spmatrix_uint *csr = gsl_spmatrix_uint_compress(graph->t, GSL_SPMATRIX_CSR);
        gsl_spmatrix_uint_free(graph->t);
        graph->t = csr;
    }
}

// Create a triangular tiling of width m
struct graph_t *graph_create_triangular(unsigned int m)
{
    unsigned int num_vertices = 3 * m * m - 3 * m + 1;
    unsigned int num_edges = 9 * m * m - 15 * m + 6;
    unsigned int num_row = 2 * m - 1;

    struct graph_t *graph = graph_create(num_vertices);
    if (!graph)
        return NULL;

    graph->type = TRIANGULAR;

    // Add edges for the top half of the tiling
    unsigned int vertex = 0;
    for (unsigned int row = 0; row < m - 1; row++)
    {
        for (unsigned int col = 0; col < m + row; col++)
        {
            unsigned int current = vertex++;

            // Horizontal edge (east)
            if (col < m + row - 1)
            {
                graph_add_edge(graph, (struct edge_t){current, current + 1}, E);
            }

            // Diagonal edge (south-east)
            graph_add_edge(graph, (struct edge_t){current, current + m + row + 1}, SE);

            // Diagonal edge (south-west)
            graph_add_edge(graph, (struct edge_t){current, current + m + row}, SW);
        }
    }

    // Add edges for the middle row of the tiling
    for (unsigned int col = 0; col < num_row; col++)
    {
        unsigned int current = vertex++;

        // Horizontal edge (east)
        if (col < num_row - 1)
        {
            graph_add_edge(graph, (struct edge_t){current, current + 1}, E);
        }
    }

    // Add edges for the bottom half of the tiling
    for (unsigned int row = m; row < num_row; row++)
    {
        for (unsigned int col = 0; col < m + num_row - row - 1; col++)
        {
            unsigned int current = vertex++;

            // Horizontal edge (east)
            if (col < m + num_row - row - 2)
            {
                graph_add_edge(graph, (struct edge_t){current, current + 1}, E);
            }

            // Diagonal edge (north-east)
            graph_add_edge(graph, (struct edge_t){current, current - (m + num_row - row) + 1}, NE);

            // Diagonal edge (north-west)
            graph_add_edge(graph, (struct edge_t){current, current - (m + num_row - row)}, NW);
        }
    }

    // Validate the number of edges added
    if (graph->num_edges != num_edges)
    {
        printf("Error: Added edges count (%u) differs from expected "
               "(%u)\n",
               graph->num_edges, num_edges);
    }

    graph_ensure_csr_format(graph);
    return graph;
}

struct graph_t *graph_create_cyclic(unsigned int m)
{
    unsigned int num_edges = 24 * m - 36;
    unsigned int num_row = 2 * m - 1;

    struct graph_t *graph = graph_create_triangular(m);
    if (!graph)
        return NULL;

    graph->type = CYCLIC;

    unsigned int vertex = 2 * m + 2;
    unsigned int vertex_middle;

    // Remove edges from the first half
    for (unsigned int row = 2; row < m; row++)
    {

        if (row == m - 1)
        {
            vertex_middle = vertex;
        }

        for (unsigned int col = 1; col < m + row - 2; col++)
        {
            unsigned int current = vertex++;

            if (col == 1)
            {
                if (gsl_spmatrix_uint_get(graph->t, current, current + 1) != NO_EDGE)
                {
                    manual_edge_set(graph, (struct edge_t){current, current + 1}, NO_EDGE);
                    graph->num_edges--;
                }
            }

            else
            {

                // Horizontal edge (east)
                if (gsl_spmatrix_uint_get(graph->t, current, current + 1) != NO_EDGE)
                {
                    manual_edge_set(graph, (struct edge_t){current, current + 1}, NO_EDGE);
                    graph->num_edges--;
                }

                // Diagonal edge (north-east)
                if (gsl_spmatrix_uint_get(graph->t, current, current - m - row + 1) != NO_EDGE)
                {
                    manual_edge_set(graph, (struct edge_t){current, current - m - row + 1},
                                    NO_EDGE);
                    graph->num_edges--;
                }

                // Diagonal edge (north-west)
                if (gsl_spmatrix_uint_get(graph->t, current, current - m - row) != NO_EDGE)
                {
                    manual_edge_set(graph, (struct edge_t){current, current - m - row}, NO_EDGE);
                    graph->num_edges--;
                }
            }
        }
        vertex += 3;
    }

    vertex = vertex_middle;

    // Remove edges from the second half
    for (unsigned int row = m - 1; row < num_row - 2; row++)
    {
        for (unsigned int col = 1; col < m + num_row - row - 3; col++)
        {
            unsigned int current = vertex++;

            if (col == 1)
            {
                if (gsl_spmatrix_uint_get(graph->t, current, current + 1) != NO_EDGE)
                {
                    manual_edge_set(graph, (struct edge_t){current, current + 1}, NO_EDGE);
                    graph->num_edges--;
                }
            }

            else
            {
                // Horizontal edge (east)
                if (gsl_spmatrix_uint_get(graph->t, current, current + 1) != NO_EDGE)
                {
                    manual_edge_set(graph, (struct edge_t){current, current + 1}, NO_EDGE);
                    graph->num_edges--;
                }

                // Diagonal edge (south-east)
                if (gsl_spmatrix_uint_get(graph->t, current, current + m + num_row - row - 1) !=
                    NO_EDGE)
                {
                    manual_edge_set(
                        graph, (struct edge_t){current, current + m + num_row - row - 1}, NO_EDGE);
                    graph->num_edges--;
                }

                // Diagonal edge (south-west)
                if (gsl_spmatrix_uint_get(graph->t, current, current + m + num_row - row - 2) !=
                    NO_EDGE)
                {
                    manual_edge_set(
                        graph, (struct edge_t){current, current + m + num_row - row - 2}, NO_EDGE);
                    graph->num_edges--;
                }
            }
        }
        vertex += 3;
    }

    // Validate the number of edges added
    if (graph->num_edges != num_edges)
    {
        printf("Error: Added edges count (%u) differs from expected "
               "(%u)\n",
               graph->num_edges, num_edges);
    }

    return graph;
}

// Get the neighbors of a vertex
unsigned int *graph_get_neighbors(struct graph_t *graph, unsigned int vertex, unsigned int *count)
{
    if (!graph || vertex >= graph->num_vertices || !count)
    {
        *count = 0;
        return NULL;
    }

    // Ensure the matrix is in CSR format for efficient iteration
    graph_ensure_csr_format(graph);

    // Count neighbors
    *count = 0;
    unsigned int max_k = graph->t->p[vertex + 1];
    for (unsigned int k = graph->t->p[vertex]; k < max_k; k++)
    {
        if (graph->t->data[k] != NO_EDGE && graph->t->data[k] != WALL_DIR)
        {
            (*count)++;
        }
    }

    if (*count == 0)
        return NULL;

    // Allocate the neighbor array
    unsigned int *neighbors = malloc(*count * sizeof(unsigned int));
    if (!neighbors)
    {
        *count = 0;
        return NULL;
    }

    // Fill the neighbor array
    unsigned int idx = 0;
    for (unsigned int k = graph->t->p[vertex]; k < max_k; k++)
    {
        if (graph->t->data[k] != NO_EDGE && graph->t->data[k] != WALL_DIR)
        {
            neighbors[idx++] = graph->t->i[k];
        }
    }

    return neighbors;
}

// Breadth-first search to check if a path exists
bool graph_has_path(struct graph_t *graph, unsigned int from, unsigned int to)
{
    if (!graph || from >= graph->num_vertices || to >= graph->num_vertices)
    {
        return false;
    }

    // Trivial case
    if (from == to)
        return true;

    // Array to mark visited vertices
    bool *visited = calloc(graph->num_vertices, sizeof(bool));
    if (!visited)
        return false;

    // Queue for BFS traversal
    unsigned int *queue = malloc(graph->num_vertices * sizeof(unsigned int));
    if (!queue)
    {
        free(visited);
        return false;
    }

    unsigned int front = 0, rear = 0;

    // Enqueue the start vertex
    queue[rear++] = from;
    visited[from] = true;

    bool path_found = false;

    // BFS traversal
    while (front < rear && !path_found)
    {
        unsigned int current = queue[front++];

        // Retrieve neighbors
        unsigned int neighbor_count;
        unsigned int *neighbors = graph_get_neighbors(graph, current, &neighbor_count);

        if (neighbors)
        {
            for (unsigned int i = 0; i < neighbor_count; i++)
            {
                unsigned int neighbor = neighbors[i];

                if (neighbor == to)
                {
                    path_found = true;
                    break;
                }

                if (!visited[neighbor])
                {
                    visited[neighbor] = true;
                    queue[rear++] = neighbor;
                }
            }

            free(neighbors);
        }
    }

    free(visited);
    free(queue);

    return path_found;
}

// Print the full adjacency matrix (CSR structure expanded for readability)
void print_adjacency_matrix(struct graph_t *graph)
{
    if (!graph)
    {
        printf("Invalid graph!\n");
        return;
    }

    printf("Adjacency matrix (%u x %u):\n", graph->num_vertices, graph->num_vertices);

    // Print column indices
    printf("    ");
    for (unsigned int j = 0; j < graph->num_vertices; j++)
    {
        printf("%2u ", j);
    }
    printf("\n");

    // Separator line
    printf("   ");
    for (unsigned int j = 0; j < graph->num_vertices; j++)
    {
        printf("---");
    }
    printf("\n");

    // Print each row of the matrix
    for (unsigned int i = 0; i < graph->num_vertices; i++)
    {
        printf("%2u |", i); // Row index

        for (unsigned int j = 0; j < graph->num_vertices; j++)
        {
            // Retrieve the matrix value from the CSR representation
            unsigned int val = NO_EDGE;

            // Iterate over non-zero elements of row i
            unsigned int max_k = graph->t->p[i + 1];
            for (unsigned int k = graph->t->p[i]; k < max_k; k++)
            {
                unsigned int col = graph->t->i[k];
                if (col == j)
                {
                    val = graph->t->data[k];
                    break;
                }
            }

            printf("%2u ", val);
        }
        printf("\n");
    }
}

// Print graph details
void print_graph_info(struct graph_t *graph)
{
    if (!graph)
    {
        printf("Invalid graph!\n");
        return;
    }

    printf("Graph with %u vertices and %u edges\n", graph->num_vertices, graph->num_edges);
    printf("Matrix format: %s\n", gsl_spmatrix_uint_type(graph->t));

    // Display edges
    printf("Edge list:\n");
    for (unsigned int i = 0; i < graph->num_vertices; i++)
    {
        unsigned int max_k = graph->t->p[i + 1];
        for (unsigned int k = graph->t->p[i]; k < max_k; k++)
        {
            unsigned int j = graph->t->i[k];
            enum dir_t dir = graph->t->data[k];
            if (dir != NO_EDGE && i < j)
            { // Avoid printing each edge twice
                printf("  (%u, %u) - Direction: %d\n", i, j, dir);
            }
        }
    }
}
