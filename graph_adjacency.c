#include "graph.h"

/*
 * [Funcionalidade 10] Impressao das listas de adjacencias do grafo.
 */

void graph_adjacency_list(char *bin_filename) {
  Graph *g = build_graph(bin_filename);
  if (!g) { printf(GRAPH_ERR_MSG); return; }

  // Para cada vertice (ja em ordem crescente de nome), imprime o nome da
  // estacao seguido de sua lista de arestas. Cada valor e separado por ", ".
  for (int i = 0; i < g->num_vertices; i++) {
    printf("%s", g->vertices[i].name);
    for (Edge *e = g->vertices[i].edges; e; e = e->next) {
      printf(", %s, %d", e->dest_name, e->distance);
      for (int j = 0; j < e->num_lines; j++)
        printf(", %s", e->lines[j]);
    }
    printf("\n");
  }

  delete_graph(&g);
}
