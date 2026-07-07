#include "graph.h"
#include "header.h"
#include "record.h"
#include "utils.h"

// Rotulo usado nas arestas de integracao (linha "Integracao", acentuada).
#define LINE_INTEGRATION "Integração"

// Valor "infinito" para o algoritmo de caminho minimo. Vale 1e9: maior que
// qualquer soma de distancias possivel neste conjunto de dados e ainda dentro
// do intervalo de um "long" de 32 bits (portavel entre Windows e Linux).
#define INF 1000000000L

// Mensagem de erro padrao das funcionalidades 10 a 13.
#define ERR_MSG "Falha na execução da funcionalidade.\n"

// ─────────────────────────────────────────────────────────────────────────────
//  Estruturas internas do grafo (listas de adjacencias)
// ─────────────────────────────────────────────────────────────────────────────

/*
 * Modelo de vertices: apenas as estacoes que sao ORIGEM de pelo menos uma
 * aresta compoem o vetor de vertices (V). Estacoes que aparecem somente como
 * destino (por exemplo, estacoes terminais de uma linha, sem codProxEstacao)
 * nao viram vertices: elas continuam sendo exibidas como destino nas listas de
 * adjacencia da funcionalidade 10, mas nao sao percorridas pelos algoritmos
 * (arvore geradora minima, caminho minimo e ciclos).
 */

/**
 * Aresta da lista de adjacencias. Guarda o nome do destino (para a impressao da
 * funcionalidade 10, pois o destino pode nao ser um vertice) e tambem o indice
 * do vertice de destino em V (dest_idx), ou -1 quando o destino nao e vertice.
 */
typedef struct edge_st {
  char *dest_name;       // nome da estacao destino (v)
  int dest_idx;          // indice do destino em V, ou -1 se destino nao e vertice
  int distance;          // peso da aresta (distancia entre as estacoes)
  char **lines;          // nomes das linhas associadas (ordenados e sem repeticao)
  int num_lines;         // quantidade de nomes de linha
  struct edge_st *next;  // proxima aresta da lista (ordenada por nome do destino)
} Edge;

/**
 * Vertice do grafo: nome da estacao e a lista encadeada de arestas que saem
 * dele, mantida ordenada de forma crescente pelo nome da estacao de destino.
 */
typedef struct {
  char *name;   // nome da estacao
  Edge *edges;  // lista de arestas de saida
} Vertex;

/**
 * Grafo direcionado ponderado: vetor de vertices ordenado por nome de estacao.
 */
typedef struct {
  Vertex *vertices;   // vetor de |V| vertices ordenado crescentemente por nome
  int num_vertices;   // quantidade de vertices |V|
} Graph;

// ─────────────────────────────────────────────────────────────────────────────
//  Funcoes auxiliares de construcao
// ─────────────────────────────────────────────────────────────────────────────

// Comparador de strings para qsort (ordena nomes de estacao crescentemente).
static int cmp_str(const void *a, const void *b) {
  return strcmp(*(const char **)a, *(const char **)b);
}

/**
 * Busca binaria de um vertice pelo nome, aproveitando que o vetor de vertices
 * esta ordenado crescentemente por nome.
 * @return indice do vertice, ou -1 se nao existir.
 */
static int find_vertex(Graph *g, const char *name) {
  int lo = 0, hi = g->num_vertices - 1;
  while (lo <= hi) {
    int mid = (lo + hi) / 2;
    int c = strcmp(g->vertices[mid].name, name);
    if (c == 0) return mid;
    if (c < 0) lo = mid + 1;
    else hi = mid - 1;
  }
  return -1;
}

/**
 * Insere um nome de linha em uma aresta, mantendo o vetor de linhas ordenado
 * crescentemente e sem repeticoes. Nomes nulos sao ignorados.
 */
static void edge_add_line(Edge *e, const char *line) {
  if (!line) return;

  // Procura a posicao de insercao (mantendo ordem) e detecta duplicatas.
  int pos = e->num_lines;
  for (int i = 0; i < e->num_lines; i++) {
    int c = strcmp(e->lines[i], line);
    if (c == 0) return;      // linha ja presente: nada a fazer
    if (c > 0) { pos = i; break; } // ponto de insercao encontrado
  }

  e->lines = realloc(e->lines, (e->num_lines + 1) * sizeof(char *));
  // Desloca os elementos a direita para abrir espaco na posicao pos.
  for (int i = e->num_lines; i > pos; i--)
    e->lines[i] = e->lines[i - 1];
  e->lines[pos] = strdup(line);
  e->num_lines++;
}

/**
 * Adiciona uma aresta u -> dest_name ao grafo. Se ja existir uma aresta para
 * esse destino, apenas agrega o nome da linha (varios registros com o mesmo par
 * (u, v) representam a mesma aresta com multiplas linhas). Caso contrario, cria
 * a aresta na posicao correta para manter a lista ordenada pelo nome do destino.
 */
static void add_edge(Graph *g, int u, const char *dest_name, int dist, const char *line) {
  Edge *prev = NULL, *cur = g->vertices[u].edges;
  while (cur && strcmp(cur->dest_name, dest_name) < 0) { prev = cur; cur = cur->next; }

  if (cur && strcmp(cur->dest_name, dest_name) == 0) { // aresta ja existe
    edge_add_line(cur, line);
    return;
  }

  Edge *e = malloc(sizeof(Edge));
  e->dest_name = strdup(dest_name);
  e->dest_idx = -1;   // resolvido depois que V estiver completo
  e->distance = dist;
  e->lines = NULL;
  e->num_lines = 0;
  e->next = cur;
  if (prev) prev->next = e;
  else g->vertices[u].edges = e;

  edge_add_line(e, line);
}

/**
 * Le todos os registros ativos do arquivo de dados e constroi o grafo
 * direcionado ponderado em memoria. Retorna NULL em caso de erro (arquivo
 * inexistente ou inconsistente), permitindo que o chamador emita a mensagem de
 * falha apropriada.
 *
 * Regras de construcao das arestas (para cada registro ativo com estacao u):
 *  - Se codProxEstacao e valido e existe registro com esse codigo: aresta
 *    u -> nome(codProxEstacao), com distancia distProxEstacao e linha nomeLinha.
 *  - Se codEstIntegra e valido e aponta para estacao de nome diferente: aresta
 *    u -> nome(codEstIntegra), com distancia 0 e linha "Integracao".
 *
 * O conjunto de vertices e formado apenas pelas estacoes que sao origem de ao
 * menos uma aresta.
 */
static Graph *build_graph(char *bin_filename) {
  FILE *bin = fopen(bin_filename, "rb");
  if (!bin) return NULL;

  // Valida o cabecalho do arquivo de dados.
  Header *header = new_header();
  read_header_binary(bin, header);
  if (get_header_status(header) == '0') {
    delete_header(&header);
    fclose(bin);
    return NULL;
  }
  delete_header(&header);

  // 1. Leitura de todos os registros ativos para vetores temporarios.
  int cap = 64, n = 0, max_code = -1;
  int *code = malloc(cap * sizeof(int));      // codEstacao
  char **name = malloc(cap * sizeof(char *)); // nomeEstacao
  int *pcode = malloc(cap * sizeof(int));     // codProxEstacao
  int *pdist = malloc(cap * sizeof(int));     // distProxEstacao
  char **line = malloc(cap * sizeof(char *)); // nomeLinha
  int *icode = malloc(cap * sizeof(int));     // codEstIntegra

  fseek(bin, HEADER_SIZE, SEEK_SET);
  Record *r = new_record();
  while (read_record_binary(bin, r)) {
    if (is_removed(r)) continue;
    char *sname = get_station_name(r);
    if (!sname) continue; // registro ativo sem nome de estacao: ignorado

    if (n >= cap) {
      cap *= 2;
      code  = realloc(code,  cap * sizeof(int));
      name  = realloc(name,  cap * sizeof(char *));
      pcode = realloc(pcode, cap * sizeof(int));
      pdist = realloc(pdist, cap * sizeof(int));
      line  = realloc(line,  cap * sizeof(char *));
      icode = realloc(icode, cap * sizeof(int));
    }

    code[n]  = get_station_code(r);
    name[n]  = strdup(sname);
    pcode[n] = get_next_station_code(r);
    pdist[n] = get_next_station_distance(r);
    char *lname = get_line_name(r);
    line[n]  = lname ? strdup(lname) : NULL;
    icode[n] = get_integration_station_code(r);
    if (code[n] > max_code) max_code = code[n];
    n++;
  }
  delete_record(&r);
  fclose(bin);

  // 2. Mapa codEstacao -> nome da estacao (para resolver os destinos das
  //    arestas). Cobre todas as estacoes com registro, inclusive terminais.
  char **code_to_name = NULL;
  if (max_code >= 0) {
    code_to_name = calloc(max_code + 1, sizeof(char *)); // NULL = codigo inexistente
    for (int i = 0; i < n; i++)
      if (code[i] >= 0 && code[i] <= max_code)
        code_to_name[code[i]] = name[i];
  }

  // Helper local: resolve um codigo de estacao para o nome, ou NULL.
  #define RESOLVE(c) ((c) >= 0 && (c) <= max_code ? code_to_name[c] : NULL)

  // 3. Conjunto de vertices V = nomes das estacoes que sao ORIGEM de alguma
  //    aresta (possuem codProxEstacao resolvivel ou codEstIntegra resolvivel
  //    para nome diferente). Coleta os nomes-origem, ordena e remove duplicatas.
  char **src = malloc((n > 0 ? n : 1) * sizeof(char *));
  int ns = 0;
  for (int i = 0; i < n; i++) {
    char *pname = RESOLVE(pcode[i]);
    char *iname = RESOLVE(icode[i]);
    boolean has_edge = (pname != NULL) ||
                       (iname != NULL && strcmp(iname, name[i]) != 0);
    if (has_edge) src[ns++] = name[i];
  }
  qsort(src, ns, sizeof(char *), cmp_str);

  Graph *g = malloc(sizeof(Graph));
  g->vertices = malloc((ns > 0 ? ns : 1) * sizeof(Vertex));
  g->num_vertices = 0;
  for (int i = 0; i < ns; i++) {
    if (i == 0 || strcmp(src[i], src[i - 1]) != 0) { // pula duplicatas
      g->vertices[g->num_vertices].name = strdup(src[i]);
      g->vertices[g->num_vertices].edges = NULL;
      g->num_vertices++;
    }
  }
  free(src);

  // 4. Criacao das arestas, percorrendo os registros na ordem de leitura (RRN).
  for (int i = 0; i < n; i++) {
    int u = find_vertex(g, name[i]);
    if (u < 0) continue; // registro sem arestas: sua estacao nao e vertice

    char *pname = RESOLVE(pcode[i]);
    if (pname) add_edge(g, u, pname, pdist[i], line[i]);

    char *iname = RESOLVE(icode[i]);
    if (iname && strcmp(iname, name[i]) != 0)
      add_edge(g, u, iname, 0, LINE_INTEGRATION);
  }

  // 5. Resolve o indice em V de cada destino (fica -1 se o destino nao e vertice).
  for (int i = 0; i < g->num_vertices; i++)
    for (Edge *e = g->vertices[i].edges; e; e = e->next)
      e->dest_idx = find_vertex(g, e->dest_name);

  #undef RESOLVE

  // Libera os vetores temporarios (as strings ja foram copiadas para o grafo).
  for (int i = 0; i < n; i++) { free(name[i]); free(line[i]); }
  free(code); free(name); free(pcode); free(pdist); free(line); free(icode);
  free(code_to_name);

  return g;
}

/**
 * Libera toda a memoria associada ao grafo (vertices, arestas, nomes de destino
 * e nomes de linha).
 */
static void delete_graph(Graph **g) {
  if (!g || !*g) return;
  for (int i = 0; i < (*g)->num_vertices; i++) {
    Edge *e = (*g)->vertices[i].edges;
    while (e) {
      Edge *next = e->next;
      for (int j = 0; j < e->num_lines; j++) free(e->lines[j]);
      free(e->lines);
      free(e->dest_name);
      free(e);
      e = next;
    }
    free((*g)->vertices[i].name);
  }
  free((*g)->vertices);
  free(*g);
  *g = NULL;
}

// ─────────────────────────────────────────────────────────────────────────────
//  [10] Impressao das listas de adjacencias
// ─────────────────────────────────────────────────────────────────────────────

void graph_adjacency_list(char *bin_filename) {
  Graph *g = build_graph(bin_filename);
  if (!g) { printf(ERR_MSG); return; }

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

// ─────────────────────────────────────────────────────────────────────────────
//  [11] Caminho mais curto (Dijkstra)
// ─────────────────────────────────────────────────────────────────────────────

void graph_shortest_path(char *bin_filename, char *origin, char *dest) {
  Graph *g = build_graph(bin_filename);
  if (!g) { printf(ERR_MSG); return; }

  int s = find_vertex(g, origin);
  int t = find_vertex(g, dest);
  int nv = g->num_vertices;

  // Origem ou destino inexistentes (como vertice): nao ha caminho possivel.
  if (s < 0 || t < 0) {
    printf("Não existe caminho entre as estações solicitadas.\n");
    delete_graph(&g);
    return;
  }

  long *dist = malloc(nv * sizeof(long));
  int *prev = malloc(nv * sizeof(int));
  char *visited = calloc(nv, sizeof(char));
  for (int i = 0; i < nv; i++) { dist[i] = INF; prev[i] = -1; }
  dist[s] = 0;

  // Laco principal de Dijkstra. Como o vetor de vertices e ordenado por nome,
  // percorrer os indices em ordem crescente ao escolher o proximo vertice de
  // menor distancia resolve automaticamente o empate pelo menor nome (i).
  for (int iter = 0; iter < nv; iter++) {
    int u = -1;
    long best = INF;
    for (int i = 0; i < nv; i++)
      if (!visited[i] && dist[i] < best) { best = dist[i]; u = i; }
    if (u < 0) break; // restantes inalcancaveis
    visited[u] = 1;

    // Relaxa as arestas de saida de u (ignorando destinos que nao sao vertices).
    // A atualizacao ocorre apenas com melhora estrita; como u e finalizado em
    // ordem de menor distancia (e, em empate, menor nome), o predecessor
    // escolhido respeita as regras de desempate.
    for (Edge *e = g->vertices[u].edges; e; e = e->next) {
      if (e->dest_idx < 0) continue;
      long nd = dist[u] + e->distance;
      if (nd < dist[e->dest_idx]) { dist[e->dest_idx] = nd; prev[e->dest_idx] = u; }
    }
  }

  if (dist[t] >= INF) {
    printf("Não existe caminho entre as estações solicitadas.\n");
  } else {
    // Reconstroi o caminho de t ate s seguindo os predecessores.
    int *path = malloc(nv * sizeof(int));
    int len = 0;
    for (int x = t; x != -1; x = prev[x]) path[len++] = x;

    // O numero de estacoes percorridas nao inclui a origem, mas inclui o destino.
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

// ─────────────────────────────────────────────────────────────────────────────
//  [12] Arvore Geradora Minima (Kruskal) + busca em profundidade
// ─────────────────────────────────────────────────────────────────────────────

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
  if (!g) { printf(ERR_MSG); return; }

  int s = find_vertex(g, origin);
  int nv = g->num_vertices;
  if (s < 0) { printf(ERR_MSG); delete_graph(&g); return; }

  // 1. Deriva a versao nao-direcionada do grafo (as linhas podem ser de ida e
  //    volta). Para cada par de vertices guarda-se a menor distancia observada
  //    em qualquer sentido. Arestas para destinos que nao sao vertices sao
  //    ignoradas. Usa-se uma matriz |V| x |V| de pesos.
  long *w = malloc((long)nv * nv * sizeof(long));
  for (long i = 0; i < (long)nv * nv; i++) w[i] = INF;
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
      if (w[u * nv + v] < INF) {
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

// ─────────────────────────────────────────────────────────────────────────────
//  [13] Contagem de ciclos simples a partir da origem
// ─────────────────────────────────────────────────────────────────────────────

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
  if (!g) { printf(ERR_MSG); return; }

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
