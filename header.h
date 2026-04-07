#ifndef HEADER_H
#define HEADER_H

#include "utils.h"
#include "record.h"

#define HEADER_SIZE 17
#define RECORD_SIZE 80
#define NAME_SIZE 43

typedef struct header_st Header;

Header* new_header();                                   // Cria e inicializa um novo header
void read_header_binary(FILE *binary, Header *header);  // Le um header de um arquivo binario
void write_header_binary(FILE *binary, Header *header); // Escreve um header em um arquivo binario
void update_header_status_binary(FILE *binary, char status);            // Atualiza o status do header no arquivo
void update_header_control_fields(Header *header, Record *new_record);  // Atualiza os campos de controle do header
void print_header(Header *header);                      // Imprime o header
void delete_header(Header **header);                    // Libera a memória de um header
char get_header_status(Header *header);                 // Retorna o status do header
#endif
