#include "graph.h"

/*
 * [Funcionalidade 13] Contagem de ciclos simples a partir da estacao de origem.
 */

/**
 * Busca em profundidade com backtracking que conta os ciclos simples que partem
 * e retornam a origem. Ao encontrar uma aresta para a origem, um ciclo e
 * contado; vertices ja no caminho atual (visited) nao sao reexplorados,
 * garantindo que nenhum vertice se repita (exceto origem = destino).
 */
static long count_cycles(Graph *g, int u, int origin, char *visited) {
  long total = 0;
  for (Edge *e = g->vertices[u].edges; e; e = e->next) {
    int v = e->dest_idx;
    if (v < 0) continue;         // destino nao e vertice: nunca fecha ciclo
    if (v == origin) {
      total++;                   // fechou um ciclo simples de volta a origem
    } else if (!visited[v]) {
      visited[v] = 1;
      total += count_cycles(g, v, origin, visited);
      visited[v] = 0;            // backtracking: libera o vertice para outros caminhos
    }
  }
  return total;
}

void graph_count_cycles(char *bin_filename, char *origin) {
  Graph *g = build_graph(bin_filename);
  if (!g) { printf(GRAPH_ERR_MSG); return; }

  int s = find_vertex(g, origin);

  // Origem sem arestas de saida (ou inexistente como vertice): nao ha como
  // sair e retornar, portanto nenhum ciclo (imprime -1).
  if (s < 0) {
    printf("Quantidade de ciclos: -1\n");
    delete_graph(&g);
    return;
  }

  char *visited = calloc(g->num_vertices, sizeof(char));
  visited[s] = 1;
  long cycles = count_cycles(g, s, s, visited);
  free(visited);

  // Se nao ha nenhum ciclo (impossivel voltar a origem), imprime -1.
  if (cycles == 0) printf("Quantidade de ciclos: -1\n");
  else printf("Quantidade de ciclos: %ld\n", cycles);

  delete_graph(&g);
}
