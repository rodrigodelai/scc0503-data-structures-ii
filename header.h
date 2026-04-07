#ifndef HEADER_H
#define HEADER_H

#include "utils.h"
#include "record.h"

#define HEADER_SIZE 17
#define RECORD_SIZE 80
#define NAME_SIZE 43

typedef struct header_st Header;

/**
 * Cria e inicializa um novo header.
 * @return Ponteiro para o novo header.
 */
Header* new_header();

/**
 * Le um header de um arquivo binario.
 * @param binary Ponteiro para o arquivo binario.
 * @param header Ponteiro para o header a ser preenchido.
 */
void read_header_binary(FILE *binary, Header *header);

/**
 * Escreve um header em um arquivo binario.
 * @param binary Ponteiro para o arquivo binario.
 * @param header Ponteiro para o header a ser escrito.
 */
void write_header_binary(FILE *binary, Header *header);

/**
 * Atualiza o status do header no arquivo.
 * @param binary Ponteiro para o arquivo binario.
 * @param status Novo status do header.
 */
void update_header_status_binary(FILE *binary, char status);

/**
 * Atualiza os campos de controle do header.
 * @param header Ponteiro para o header.
 * @param new_record Ponteiro para registro recem adicionado.
 */
void update_header_control_fields(Header *header, Record *new_record);

/**
 * Retorna o status do header.
 * @param header Ponteiro para o header.
 * @return Status do header.
 */
char get_header_status(Header *header);

/**
 * Imprime o header.
 * @param header Ponteiro para o header.
 */
void print_header(Header *header);

/**
 * Libera a memoria de um header.
 * @param header Ponteiro para o ponteiro do header.
 */
void delete_header(Header **header);

#endif
