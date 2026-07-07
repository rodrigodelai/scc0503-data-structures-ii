#include "graph.h"
#include "header.h"
#include "record.h"
#include "utils.h"

/*
 * TAD Grafo: construcao a partir do arquivo de dados, liberacao e busca de
 * vertices. As quatro funcionalidades (10 a 13) ficam em arquivos separados e
 * usam as operacoes declaradas em graph.h.
 */

// Rotulo usado nas arestas de integracao (linha "Integracao", acentuada).
#define LINE_INTEGRATION "Integração"

// ─────────────────────────────────────────────────────────────────────────────
//  Funcoes auxiliares internas de construcao
// ─────────────────────────────────────────────────────────────────────────────

// Comparador de strings para qsort (ordena nomes de estacao crescentemente).
static int cmp_str(const void *a, const void *b) {
  return strcmp(*(const char **)a, *(const char **)b);
}

int find_vertex(Graph *g, const char *name) {
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

// ─────────────────────────────────────────────────────────────────────────────
//  Construcao e liberacao do grafo
// ─────────────────────────────────────────────────────────────────────────────

/*
 * Regras de construcao das arestas (para cada registro ativo com estacao u):
 *  - Se codProxEstacao e valido e existe registro com esse codigo: aresta
 *    u -> nome(codProxEstacao), com distancia distProxEstacao e linha nomeLinha.
 *  - Se codEstIntegra e valido e aponta para estacao de nome diferente: aresta
 *    u -> nome(codEstIntegra), com distancia 0 e linha "Integracao".
 *
 * O conjunto de vertices e formado apenas pelas estacoes que sao origem de ao
 * menos uma aresta.
 */
Graph *build_graph(char *bin_filename) {
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

void delete_graph(Graph **g) {
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
