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

#endif // INDEX_H