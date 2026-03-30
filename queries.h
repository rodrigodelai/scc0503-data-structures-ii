#ifndef QUERIES_H
#define QUERIES_H

#include "header.h"
#include "record.h"
#include "utils.h"

void create_from_csv(const char *csv_filename, const char *bin_filename); // Cria um arquivo binario a partir de um arquivo csv
void select_all(const char *bin_filename); // Recupera todos os registros de um arquivo binario
void select_where(const char *bin_filename, int num_criteria); // Recupera registros de um arquivo binario que atendem aos criterios
void select_by_rrn(const char *bin_filename, int rrn); // Recupera um registro de um arquivo binario pelo rrn

#endif // QUERIES_H