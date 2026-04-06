#ifndef RECORD_H
#define RECORD_H

#include "utils.h"

#define HEADER_SIZE 17
#define RECORD_SIZE 80
#define NAME_SIZE 43

typedef enum { REMOVED = '1', NOT_REMOVED = '0' } Removed;
typedef struct record_st Record;

// Core functions
Record* new_record();                                   // Cria um novo registro
boolean read_record_csv(FILE *csv, Record *record);     // Le um registro de um arquivo csv
boolean read_record_binary(FILE *bin, Record *record);  // Le um registro de um arquivo binario
int write_record_binary(FILE *bin, Record *record);     // Escreve um registro em um arquivo binario e retorna o RRN do registro
void print_record(Record *record);                      // Imprime um registro
void print_record_one_line(Record *record);             // Imprime um registro em uma linha
void delete_record(Record **record);                    // Libera a memoria de um registro

// Getters
Removed get_removed(Record *record);
int get_next_removed_rrn(Record *record);
int get_station_code(Record *record);
int get_line_code(Record *record);
int get_next_station_code(Record *record);
int get_next_station_distance(Record *record);
int get_integration_line_code(Record *record);
int get_integration_station_code(Record *record);
int get_station_name_size(Record *record);
char* get_station_name(Record *record);
int get_line_name_size(Record *record);
char* get_line_name(Record *record);

// Setters
boolean set_removed(Record *record, Removed removed);
boolean set_next_removed_rrn(Record *record, int next_removed_rrn);
boolean set_station_code(Record *record, int station_code);
boolean set_line_code(Record *record, int line_code);
boolean set_next_station_code(Record *record, int next_station_code);
boolean set_next_station_distance(Record *record, int next_station_distance);
boolean set_integration_line_code(Record *record, int integration_line_code);
boolean set_integration_station_code(Record *record, int integration_station_code);
boolean set_station_name_size(Record *record, int station_name_size);
boolean set_station_name(Record *record, char *station_name);
boolean set_line_name_size(Record *record, int line_name_size);
boolean set_line_name(Record *record, char *line_name);

#endif
