#ifndef RECORD_H
#define RECORD_H

#include "utils.h"

#define HEADER_SIZE 17
#define RECORD_SIZE 80
#define NAME_SIZE 43

typedef struct {
  char removed;                 // 1 byte: '1' removido, '0' não removido
  int next_removed_rrn;         // 4 bytes: RRN do próximo removido ou -1
  
  int station_code;             // 4 bytes: código da estacao
  int line_code;                // 4 bytes: código da linha
  int next_station_code;        // 4 bytes: código da proxima estacao
  int next_station_distance;    // 4 bytes: distancia ate a proxima estacao
  int integration_line_code;    // 4 bytes: codigo da linha de integracao
  int integration_station_code; // 4 bytes: codigo da estacao de integracao
  
  int station_name_size;        // 4 bytes: tamanho do nome da estacao
  char *station_name;           // N bytes: nome da estacao (max 43 bytes)
  
  int line_name_size;           // 4 bytes: tamanho do nome da linha
  char *line_name;              // M bytes: nome da linha; N + M = 43 bytes
} Record;

Record* new_record();                             // Cria um novo registro
boolean read_record_csv(FILE *csv, Record *record);  // Le um registro de um arquivo csv
boolean read_record_binary(FILE *bin, Record *record);  // Le um registro de um arquivo binario
int write_record_binary(FILE *bin, Record *record);  // Escreve um registro em um arquivo binario e retorna o RRN do registro
void print_record(Record *record);                   // Imprime um registro
void print_record_one_line(Record *record);          // Imprime um registro em uma linha
void delete_record(Record **record);                 // Libera a memoria de um registro

#endif

