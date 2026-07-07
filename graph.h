#ifndef GRAPH_H
#define GRAPH_H

/*
 * Modulo de grafos (Trabalho Pratico 2, funcionalidades 10 a 13).
 *
 * Cada funcao publica abaixo constitui o ponto de entrada de uma
 * funcionalidade. Todas elas leem o arquivo de dados binario (mesmo formato
 * dos trabalhos anteriores), constroem internamente um grafo direcionado e
 * ponderado representado por listas de adjacencias, e executam a investigacao
 * correspondente, imprimindo o resultado na saida padrao.
 *
 * O grafo modela a malha de metro/CPTM: cada vertice e uma estacao (unica por
 * nome) e cada aresta (u, v) representa uma ligacao de u para v, ponderada pela
 * distancia e rotulada pelos nomes das linhas que a percorrem. Arestas de
 * integracao tem peso 0 e linha "Integracao".
 */

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
