#ifndef RECORD_H
#define RECORD_H

#include "utils.h"

#define HEADER_SIZE 17
#define RECORD_SIZE 80
#define NAME_SIZE 43

typedef struct record_st Record;

/**
 * Cria e inicializa um novo registro.
 * @return Ponteiro para o novo registro.
 */
Record* new_record();

/**
 * Le um registro de um arquivo csv.
 * @param csv Ponteiro para o arquivo csv.
 * @param record Ponteiro para o registro a ser preenchido.
 * @return true se o registro foi lido com sucesso, false caso contrario.
 */
boolean read_record_csv(FILE *csv, Record *record);

/**
 * Le um registro de um arquivo binario.
 * @param bin Ponteiro para o arquivo binario.
 * @param record Ponteiro para o registro a ser preenchido.
 * @return true se o registro foi lido com sucesso, false caso contrario.
 */
boolean read_record_binary(FILE *bin, Record *record);

/**
 * Escreve um registro em um arquivo binario e retorna o RRN do registro.
 * @param bin Ponteiro para o arquivo binario.
 * @param record Ponteiro para o registro a ser escrito.
 * @return RRN do registro escrito.
 */
int write_record_binary(FILE *bin, Record *record);

/**
 * Imprime um registro.
 * @param record Ponteiro para o registro a ser impresso.
 */
void print_record(Record *record);
void print_record_one_line(Record *record);
void delete_record(Record **record);

// Getters
boolean is_removed(Record *record);
int get_station_code(Record *record);
int get_next_station_code(Record *record);
char* get_station_name(Record *record);

#endif
