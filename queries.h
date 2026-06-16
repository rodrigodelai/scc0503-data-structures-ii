#ifndef QUERIES_H
#define QUERIES_H

#include "header.h"
#include "record.h"
#include "utils.h"


/**
 * Estrutura para armazenar critérios de busca.
 */
typedef struct {
  char name[50];
  char value[100];
} Criterion;


/**
 * Cria um arquivo binario a partir de um arquivo csv.
 * @param csv_filename Nome ou caminho do arquivo csv.
 * @param bin_filename Nome ou caminho do arquivo binario.
 */
void create_from_csv(char *csv_filename, char *bin_filename);

/**
 * Recupera todos os registros de um arquivo binario.
 * @param bin_filename Nome ou caminho do arquivo binario.
 */
void select_all(char *bin_filename);

/**
 * Recupera registros de um arquivo binario que atendem aos criterios.
 * @param bin_filename Nome ou caminho do arquivo binario.
 * @param num_criteria Numero de criterios de busca.
 */
void select_where(char *bin_filename, int num_criteria);

/**
 * Recupera um registro de um arquivo binario pelo rrn.
 * @param bin_filename Nome ou caminho do arquivo binario.
 * @param rrn RRN do registro.
 */
void select_by_rrn(char *bin_filename, int rrn);

/**
 * [Funcionalidade 6] Recupera registros otimizados com índice primário.
 * @param bin_filename Nome do arquivo de dados.
 * @param index_filename Nome do arquivo de índice primário.
 * @param num_searches Numero de buscas a serem realizadas.
 */
void select_where_indexed(char *bin_filename, char *index_filename, int num_searches);

/**
 * [Funcionalidade 7] Realiza a remoção lógica de registros.
 * @param bin_filename Nome do arquivo de dados.
 * @param index_filename Nome do arquivo de índice primário.
 * @param num_removals Número de remoções a serem realizadas.
 */
void delete_records(char *bin_filename, char *index_filename, int num_removals);
#endif // QUERIES_H
