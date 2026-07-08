#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {false, true} boolean;

int read_integer();

/**
 * Lê um inteiro que esteja no RESTANTE da linha atual da entrada padrão.
 * Pula espaços e tabs; se encontrar um dígito, lê o inteiro em *out e retorna 1.
 * Se a linha terminar (\n, \r ou EOF) antes de qualquer dígito, retorna 0 e
 * consome o fim de linha. Útil para parâmetros opcionais no fim da linha de
 * comando (ex.: o número de buscas da funcionalidade 6, que pode estar ausente).
 * @param out Onde armazenar o inteiro lido.
 * @return 1 se leu um inteiro na linha atual, 0 caso a linha tenha terminado.
 */
int read_int_same_line(int *out);

void scan_quote_string(char *str);
void fill_with_trash(char *str, int size);
void binario_na_tela(const char *arquivo);
char* read_string();
void delete_string(char **str);
char* read_line_csv(FILE *csv);
char* read_optional_string_csv(FILE *csv, char delimiter);
int read_optional_int_csv(FILE *csv, char delimiter);

#endif
