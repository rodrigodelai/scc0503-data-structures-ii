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