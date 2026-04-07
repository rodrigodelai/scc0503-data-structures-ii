#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {false, true} boolean;

int read_integer();
void scan_quote_string(char *str);
void fill_with_trash(char *str, int size);
void binario_na_tela(const char *arquivo);
char* read_string();
void delete_string(char **str);
char* read_line_csv(FILE *csv);
int read_optional_int_csv(FILE *csv, char delimiter);

#endif
