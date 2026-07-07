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
 * Busca o RRN de uma estação diretamente no vetor de índices que já está
 * carregado em RAM, usando Busca Binária (o vetor está ordenado de forma
 * crescente por codEstacao). Esta é a forma eficiente de busca: não há seeks
 * em disco, apenas acessos ao vetor em memória.
 * @param entries Vetor de entradas de índice carregado em RAM.
 * @param num_entries Número de entradas no vetor.
 * @param target_cod Codigo da estação buscada.
 * @return O RRN correspondente, ou -1 se não for encontrado.
 */
int search_index_ram(IndexEntry *entries, int num_entries, int target_cod);

// Carrega o arquivo de índice para a RAM e retorna o vetor e o número de elementos
IndexEntry* load_index(char *index_filename, int *num_entries);

// Escreve apenas o byte de status no cabeçalho do arquivo de índice em disco.
// Usado para marcar o índice como inconsistente ('0') no início de uma
// funcionalidade que o modifica, e consistente ('1') ao final.
void set_index_status_binary(char *index_filename, char status);

// Remove uma estação do vetor de índices em RAM
void remove_from_index(IndexEntry *entries, int *num_entries, int target_cod);

// Regrava o vetor de índices de volta no arquivo
void rewrite_index(char *index_filename, IndexEntry *entries, int num_entries);

// Adiciona uma nova entrada ao vetor de índices em memória e o reordena
void add_to_index(IndexEntry **entries, int *num_entries, int cod, int rrn);

#endif // INDEX_H