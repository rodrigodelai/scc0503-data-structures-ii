#include "queries.h"

void create_from_csv(char *csv_filename, char *bin_filename) {
  // open files
  FILE *csv = fopen(csv_filename, "r");
  FILE *bin = fopen(bin_filename, "wb");

  if (!csv || !bin) {
    printf("Falha no processamento do arquivo.\n");
    delete_string(&csv_filename);
    delete_string(&bin_filename);
    csv ? fclose(csv) : 0;
    bin ? fclose(bin) : 0;
    return;
  }

  // skip header info
  free(read_line_csv(csv));

  // write empty header
  Header *header = new_header();
  write_header_binary(bin, header);

  // copy records from csv to binary
  Record *record = new_record();
  
  while (read_record_csv(csv, record)) {
    write_record_binary(bin, record);
    update_header_control_fields(header, record);
  }

  write_header_binary(bin, header);
  update_header_status_binary(bin, '1');

  // cleanup
  delete_record(&record);
  delete_header(&header);
  fclose(csv);
  fclose(bin);

  // output verification
  binario_na_tela(bin_filename);
  delete_string(&csv_filename);
  delete_string(&bin_filename);
}

void select_all(char *bin_filename) {
  // open file
  FILE *bin = fopen(bin_filename, "rb");

  if (!bin) {
    printf("Falha no processamento do arquivo.\n");
    delete_string(&bin_filename);
    fclose(bin);
    return;
  }

  // read header
  Header *header = new_header();
  read_header_binary(bin, header);

  // read and print records
  Record *record = new_record();

  while (read_record_binary(bin, record)) {
    if (!is_removed(record)) {
      print_record_one_line(record);
    }
  }  

  delete_record(&record);
  delete_header(&header);
  delete_string(&bin_filename);
  fclose(bin);
}

void select_where(char *bin_filename, int num_criteria); // Recupera registros de um arquivo binario que atendem aos criterios
void select_by_rrn(char *bin_filename, int rrn); // Recupera um registro de um arquivo binario pelo rrn
