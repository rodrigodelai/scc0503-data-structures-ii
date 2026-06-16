#ifndef INDEX_H
#define INDEX_H

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "record.h"
#include "header.h"

/**
 * Estrutura que representa um registro no arquivo de índice primário em RAM.
 */
typedef struct {
    int codEstacao;
    int rrn;
} IndexEntry;

/**
 * [Funcionalidade 5] Cria um arquivo de índice primário a partir do arquivo de dados.
 * @param bin_filename Nome do arquivo de dados binário (ex: estacao.bin)
 * @param index_filename Nome do arquivo de índice a ser criado (ex: indexaEstacao.bin)
 */
void create_index(char *bin_filename, char *index_filename);

/**
 * Busca o RRN de uma estação no arquivo de índice usando Busca Binária.
 * @param index_filename Nome do arquivo de índice.
 * @param target_cod Codigo da estação buscada.
 * @return O RRN correspondente, ou -1 se não for encontrado.
 */
int search_index(char *index_filename, int target_cod);

// Carrega o arquivo de índice para a RAM e retorna o vetor e o número de elementos
IndexEntry* load_index(char *index_filename, int *num_entries);

// Remove uma estação do vetor de índices em RAM
void remove_from_index(IndexEntry *entries, int *num_entries, int target_cod);

// Regrava o vetor de índices de volta no arquivo
void rewrite_index(char *index_filename, IndexEntry *entries, int num_entries);

// Adiciona uma nova entrada ao vetor de índices em memória e o reordena
void add_to_index(IndexEntry **entries, int *num_entries, int cod, int rrn);

#endif // INDEX_H