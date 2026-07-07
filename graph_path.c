#include "graph.h"

/*
 * [Funcionalidade 11] Caminho mais curto entre duas estacoes (Dijkstra).
 */

void graph_shortest_path(char *bin_filename, char *origin, char *dest) {
  Graph *g = build_graph(bin_filename);
  if (!g) { printf(GRAPH_ERR_MSG); return; }

  int s = find_vertex(g, origin);
  int t = find_vertex(g, dest);
  int nv = g->num_vertices;

  // Origem ou destino inexistentes (como vertice): nao ha caminho possivel
  if (s < 0 || t < 0) {
    printf("Não existe caminho entre as estações solicitadas.\n");
    delete_graph(&g);
    return;
  }

  long *dist = malloc(nv * sizeof(long));
  int *prev = malloc(nv * sizeof(int));
  char *visited = calloc(nv, sizeof(char));
  for (int i = 0; i < nv; i++) { dist[i] = GRAPH_INF; prev[i] = -1; }
  dist[s] = 0;

  // Laco principal de Dijkstra. Como o vetor de vertices e ordenado por nome,
  // percorrer os indices em ordem crescente ao escolher o proximo vertice de
  // menor distancia resolve automaticamente o empate pelo menor nome (i)
  for (int iter = 0; iter < nv; iter++) {
    int u = -1;
    long best = GRAPH_INF;
    for (int i = 0; i < nv; i++)
      if (!visited[i] && dist[i] < best) { best = dist[i]; u = i; }
    if (u < 0) break; // restantes inalcancaveis
    visited[u] = 1;

    // Relaxa as arestas de saida de u (ignorando destinos que nao sao vertices)
    // A atualizacao ocorre apenas com melhora estrita; como u e finalizado em
    // ordem de menor distancia (e, em empate, menor nome), o predecessor
    // escolhido respeita as regras de desempate.
    for (Edge *e = g->vertices[u].edges; e; e = e->next) {
      if (e->dest_idx < 0) continue;
      long nd = dist[u] + e->distance;
      if (nd < dist[e->dest_idx]) { dist[e->dest_idx] = nd; prev[e->dest_idx] = u; }
    }
  }

  if (dist[t] >= GRAPH_INF) {
    printf("Não existe caminho entre as estações solicitadas.\n");
  } else {
    // Reconstroi o caminho de t ate s seguindo os predecessores.
    int *path = malloc(nv * sizeof(int));
    int len = 0;
    for (int x = t; x != -1; x = prev[x]) path[len++] = x;

    // O numero de estacoes percorridas nao inclui a origem, mas inclui o destino
    printf("Numero de estacoes que serao percorridas: %d\n", len - 1);
    printf("Distancia que sera percorrida: %ld\n", dist[t]);
    for (int i = len - 1; i >= 0; i--) {
      printf("%s", g->vertices[path[i]].name);
      if (i > 0) printf(", ");
    }
    printf("\n");
    free(path);
  }

  free(dist); free(prev); free(visited);
  delete_graph(&g);
}
