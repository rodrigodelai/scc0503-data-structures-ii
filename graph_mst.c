#include "graph.h"

/*
 * [Funcionalidade 12] Arvore Geradora Minima (Kruskal) percorrida em
 * profundidade a partir da estacao de origem.
 */

// Aresta nao-direcionada usada na construcao da AGM.
typedef struct {
  int u, v, w; // u < v (indices de vertice); w = peso
} UEdge;

// Comparador de arestas para o Kruskal: ordena por peso e, em empate, pelo
// menor vertice u e depois pelo menor vertice v (regras de desempate do TP).
static int cmp_uedge(const void *a, const void *b) {
  const UEdge *x = a, *y = b;
  if (x->w != y->w) return x->w - y->w;
  if (x->u != y->u) return x->u - y->u;
  return x->v - y->v;
}

// Union-Find (conjuntos disjuntos) com compressao de caminho.
static int uf_find(int *parent, int x) {
  while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; }
  return x;
}

// No de adjacencia da AGM (arvore nao-direcionada resultante).
typedef struct mst_adj_st {
  int v;
  int w;
  struct mst_adj_st *next;
} MstAdj;

// Insere um vizinho na lista de adjacencia da AGM, mantendo-a ordenada de forma
// crescente pelo indice (nome) do vertice, para que a DFS visite os filhos em
// ordem crescente de nome.
static void mst_add(MstAdj **adj, int u, int v, int w) {
  MstAdj *prev = NULL, *cur = adj[u];
  while (cur && cur->v < v) { prev = cur; cur = cur->next; }
  MstAdj *node = malloc(sizeof(MstAdj));
  node->v = v; node->w = w; node->next = cur;
  if (prev) prev->next = node; else adj[u] = node;
}

// Busca em profundidade sobre a AGM. Ao descer de "u" para um filho ainda nao
// visitado, imprime "nome(u), nome(filho), distancia" e recorre.
static void mst_dfs(Graph *g, MstAdj **adj, int u, char *visited) {
  visited[u] = 1;
  for (MstAdj *a = adj[u]; a; a = a->next) {
    if (!visited[a->v]) {
      printf("%s, %s, %d\n", g->vertices[u].name, g->vertices[a->v].name, a->w);
      mst_dfs(g, adj, a->v, visited);
    }
  }
}

void graph_mst_dfs(char *bin_filename, char *origin) {
  Graph *g = build_graph(bin_filename);
  if (!g) { printf(GRAPH_ERR_MSG); return; }

  int s = find_vertex(g, origin);
  int nv = g->num_vertices;
  if (s < 0) { printf(GRAPH_ERR_MSG); delete_graph(&g); return; }

  // 1. Deriva a versao nao-direcionada do grafo (as linhas podem ser de ida e
  //    volta). Para cada par de vertices guarda-se a menor distancia observada
  //    em qualquer sentido. Arestas para destinos que nao sao vertices sao
  //    ignoradas. Usa-se uma matriz |V| x |V| de pesos.
  long *w = malloc((long)nv * nv * sizeof(long));
  for (long i = 0; i < (long)nv * nv; i++) w[i] = GRAPH_INF;
  for (int u = 0; u < nv; u++)
    for (Edge *e = g->vertices[u].edges; e; e = e->next) {
      int v = e->dest_idx;
      if (v < 0) continue;
      if (e->distance < w[u * nv + v]) {
        w[u * nv + v] = e->distance;
        w[v * nv + u] = e->distance; // simetrico (nao-direcionado)
      }
    }

  // 2. Coleta as arestas nao-direcionadas (u < v) existentes.
  int cap = 64, ne = 0;
  UEdge *edges = malloc(cap * sizeof(UEdge));
  for (int u = 0; u < nv; u++)
    for (int v = u + 1; v < nv; v++)
      if (w[u * nv + v] < GRAPH_INF) {
        if (ne >= cap) { cap *= 2; edges = realloc(edges, cap * sizeof(UEdge)); }
        edges[ne].u = u; edges[ne].v = v; edges[ne].w = (int)w[u * nv + v];
        ne++;
      }
  free(w);

  // 3. Kruskal: ordena as arestas pelas regras de desempate e as adiciona a
  //    AGM enquanto nao formarem ciclo (union-find).
  qsort(edges, ne, sizeof(UEdge), cmp_uedge);
  int *parent = malloc(nv * sizeof(int));
  for (int i = 0; i < nv; i++) parent[i] = i;

  MstAdj **adj = calloc(nv, sizeof(MstAdj *));
  for (int i = 0; i < ne; i++) {
    int ru = uf_find(parent, edges[i].u);
    int rv = uf_find(parent, edges[i].v);
    if (ru != rv) {
      parent[ru] = rv;
      mst_add(adj, edges[i].u, edges[i].v, edges[i].w);
      mst_add(adj, edges[i].v, edges[i].u, edges[i].w);
    }
  }
  free(edges);
  free(parent);

  // 4. Percorre a AGM em profundidade a partir da origem, imprimindo as arestas.
  char *visited = calloc(nv, sizeof(char));
  mst_dfs(g, adj, s, visited);
  free(visited);

  // Libera as listas de adjacencia da AGM.
  for (int i = 0; i < nv; i++) {
    MstAdj *a = adj[i];
    while (a) { MstAdj *next = a->next; free(a); a = next; }
  }
  free(adj);

  delete_graph(&g);
}
