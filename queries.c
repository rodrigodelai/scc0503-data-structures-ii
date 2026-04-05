#include "queries.h"

void create_from_csv(const char *csv_filename, const char *bin_filename) {
  // open files
  FILE *csv = fopen(csv_filename, "r");
  FILE *bin = fopen(bin_filename, "wb");

  if (!csv) {
    printf("Error opening file under path '%s'", csv_filename);
    return;
  }

  if (!bin) {
    printf("Error opening file under path '%s'", bin_filename);
    return;
  }

  // skip header info
  free(read_line_csv(csv));

  // write empty header
  Header *header = new_header();
  write_header_binary(bin, header);
  delete_header(&header);

  // copy records
  Record *record = new_record();

  while (read_record_csv(csv, record))
    write_record_binary(bin, record);
  
  delete_record(&record);
  fclose(csv);
  
  // copy csv header (workaround till header logic is implemented)
  FILE *check = fopen("./provided-files/estacoes.bin", "rb");
  if (!check) {
    printf("Error opening file under path './provided-files/estacoes.bin'");
    return;
  }
  
  Header *copy = new_header();
  read_header_binary(check, copy);
  write_header_binary(bin, copy);
  delete_header(&copy);

  fclose(check);
  fclose(bin);

  // output verification
  binario_na_tela(bin_filename);
}

void select_all(const char *bin_filename); // Recupera todos os registros de um arquivo binario
void select_where(const char *bin_filename, int num_criteria); // Recupera registros de um arquivo binario que atendem aos criterios
void select_by_rrn(const char *bin_filename, int rrn); // Recupera um registro de um arquivo binario pelo rrn