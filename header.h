#ifndef HEADER_H
#define HEADER_H

#include "utils.h"
#include "record.h"

#define HEADER_SIZE 17
#define RECORD_SIZE 80
#define NAME_SIZE 43

// Abstract type to map presence/uniqueness of stations
typedef struct {
  int capacity;
  int size;
  char** stations;
} StationsArray;

// Abstract type to map presence/uniqueness of station pairs
typedef struct {
  boolean present[1000][1000];
} PairsArray;

// Header structure for binary file + Control fields
typedef struct {
  char status;           // 1 byte: '0' inconsistente, '1' consistente
  int top_rrn;           // 4 bytes: RRN do ultimo registro removido
  int next_rrn;          // 4 bytes: proximo RRN disponivel
  int stations;          // 4 bytes: quantidade de estações
  int pairs;             // 4 bytes: quantidade de pares

  StationsArray* stations_array; // Array de para identificar estacoes unicas
  PairsArray* pairs_array;       // Array de para identificar pares de estacoes
} Header;

Header* new_header();                                   // Cria e inicializa um novo header
void read_header_binary(FILE *binary, Header *header);  // Le um header de um arquivo binario
void write_header_binary(FILE *binary, Header *header); // Escreve um header em um arquivo binario
void update_header_status_binary(FILE *binary, char status);   // Atualiza o status do header no arquivo
void update_header_control_fields(Header *header, Record *new_record); // Atualiza os campos de controle do header
void print_header(Header *header);                      // Imprime o header
void delete_header(Header **header);                    // Libera a memória de um header

StationsArray* new_stations_array();
void populate_stations_array(StationsArray *stations, FILE* binary_file); // Preenche o array com as estações do arquivo
boolean include_station(StationsArray *stations, const char* name);          // Adiciona uma estação ao array
int count_stations(StationsArray *stations);                              // Retorna a quantidade de estações
boolean is_station_present(StationsArray *stations, const char* name);    // Verifica se uma estação está presente
void delete_stations_array(StationsArray **stations);                     // Libera a memória do array

PairsArray* new_pairs_array();
void populate_pairs_array(PairsArray *pairs, FILE* binary_file);                    // Preenche o array com os pares de estações do arquivo
boolean include_pair(PairsArray *pairs, int station_code, int next_station_code);      // Adiciona um par ao array
int count_pairs(PairsArray *pairs);                                                 // Retorna a quantidade de pares
boolean is_pair_present(PairsArray *pairs, int station_code, int next_station_code); // Verifica se um par está presente
void delete_pairs_array(PairsArray **pairs);                                        // Libera a memória do array

#endif
