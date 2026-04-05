#ifndef HEADER_H
#define HEADER_H

#include "utils.h"

typedef struct {
  char status;           // 1 byte: '0' inconsistente, '1' consistente
  int top_rrn;           // 4 bytes: RRN do ultimo registro removido
  int next_rrn;          // 4 bytes: proximo RRN disponivel
  int stations;          // 4 bytes: quantidade de estações
  int pairs;             // 4 bytes: quantidade de pares
} Header;

Header* new_header();                                   // Cria e inicializa um novo header
void read_header_binary(FILE *binary, Header *header);  // Le um header de um arquivo binario
void write_header_binary(FILE *binary, Header *header); // Escreve um header em um arquivo binario
void update_header_status(FILE *binary, char status);   // Atualiza o status do header no arquivo
void print_header(Header *header);
void delete_header(Header **header);              // Libera a memória de um header

#endif
