#ifndef GRAPH_H
#define GRAPH_H

#include "utils.h"

/*
 * Modulo de grafos (Trabalho Pratico 2, funcionalidades 10 a 13).
 *
 * Este cabecalho reune o Tipo Abstrato de Dados (TAD) Grafo e as declaracoes
 * das quatro funcionalidades. Cada funcionalidade e implementada em um arquivo
 * proprio (graph_adjacency.c, graph_path.c, graph_mst.c e graph_cycles.c),
 * enquanto a construcao e a manipulacao basica do grafo ficam em graph.c.
 *
 * O grafo modela a malha de metro/CPTM: cada vertice e uma estacao e cada
 * aresta (u, v) representa uma ligacao de u para v, ponderada pela distancia e
 * rotulada pelos nomes das linhas que a percorrem. Arestas de integracao tem
 * peso 0 e linha "Integracao".
 *
 * Modelo de vertices: apenas as estacoes que sao ORIGEM de pelo menos uma
 * aresta compoem o vetor de vertices (V). Estacoes que aparecem somente como
 * destino (por exemplo, estacoes terminais de uma linha, sem codProxEstacao)
 * nao viram vertices: continuam sendo exibidas como destino nas listas de
 * adjacencia da funcionalidade 10, mas nao sao percorridas pelos algoritmos.
 */

// Valor "infinito" para os algoritmos. Vale 1e9: maior que qualquer soma de
// distancias possivel neste conjunto de dados e ainda dentro do intervalo de um
// "long" de 32 bits (portavel entre Windows e Linux).
#define GRAPH_INF 1000000000L

// Mensagem de erro padrao das funcionalidades 10 a 13.
#define GRAPH_ERR_MSG "Falha na execução da funcionalidade.\n"

// ─────────────────────────────────────────────────────────────────────────────
//  Estruturas do grafo (listas de adjacencias)
// ─────────────────────────────────────────────────────────────────────────────

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
//  Operacoes basicas do TAD (implementadas em graph.c)
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Le todos os registros ativos do arquivo de dados e constroi o grafo
 * direcionado ponderado em memoria.
 * @param bin_filename Nome do arquivo de dados binario.
 * @return Grafo construido, ou NULL em caso de erro (arquivo inexistente ou
 *         inconsistente).
 */
Graph *build_graph(char *bin_filename);

/**
 * Libera toda a memoria associada ao grafo e anula o ponteiro.
 * @param g Endereco do ponteiro para o grafo.
 */
void delete_graph(Graph **g);

/**
 * Busca binaria de um vertice pelo nome (o vetor de vertices e ordenado).
 * @return indice do vertice, ou -1 se nao existir.
 */
int find_vertex(Graph *g, const char *name);

// ─────────────────────────────────────────────────────────────────────────────
//  Pontos de entrada das funcionalidades (um arquivo por funcionalidade)
// ─────────────────────────────────────────────────────────────────────────────

/**
 * [Funcionalidade 10] Constroi o grafo e imprime as listas de adjacencias.
 * @param bin_filename Nome do arquivo de dados binario.
 */
void graph_adjacency_list(char *bin_filename);

/**
 * [Funcionalidade 11] Imprime o caminho mais curto (menor distancia total)
 * entre a estacao de origem e a de destino (algoritmo de Dijkstra).
 * @param bin_filename Nome do arquivo de dados binario.
 * @param origin Nome da estacao de origem.
 * @param dest Nome da estacao de destino.
 */
void graph_shortest_path(char *bin_filename, char *origin, char *dest);

/**
 * [Funcionalidade 12] Constroi a Arvore Geradora Minima (grafo tratado como
 * nao-direcionado) e a percorre em profundidade a partir da origem, imprimindo
 * cada aresta da arvore na ordem da busca.
 * @param bin_filename Nome do arquivo de dados binario.
 * @param origin Nome da estacao de origem.
 */
void graph_mst_dfs(char *bin_filename, char *origin);

/**
 * [Funcionalidade 13] Conta quantos ciclos simples existem a partir da estacao
 * de origem. Imprime -1 caso nao seja possivel voltar a origem (0 ciclos).
 * @param bin_filename Nome do arquivo de dados binario.
 * @param origin Nome da estacao de origem.
 */
void graph_count_cycles(char *bin_filename, char *origin);

#endif // GRAPH_H
