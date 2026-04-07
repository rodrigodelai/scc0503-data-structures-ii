#include "queries.h"

//Estrutura para armazenar critérios de busca
typedef struct {
  char name[50];
  char value[100];
} Criterion;

//Função auxiliar para verificar se um registro atende aos critérios de busca. Caso passe em todos os critérios, retorna true. Caso contrário, retorna false.
static boolean match_record(Record *r, Criterion *criteria, int m) {
  for (int i = 0; i < m; i++) {
    char *name = criteria[i].name;
    char *val = criteria[i].value;

    boolean is_null = (val[0] == '\0');

    // 1. codEstacao
    if (strcmp(name, "codEstacao") == 0) {
      if (is_null) { if (get_station_code(r) != -1) return false; }
      else { if (get_station_code(r) != atoi(val)) return false; }
    }
    // 2. nomeEstacao
    else if (strcmp(name, "nomeEstacao") == 0) {
      if (is_null) { if (get_station_name_size(r) > 0) return false; }
      else {
        char *station_name = get_station_name(r);
        if (station_name == NULL || strcmp(station_name, val) != 0) return false;
      }
    }
    // 3. codLinha
    else if (strcmp(name, "codLinha") == 0) {
      if (is_null) { if (get_line_code(r) != -1) return false; }
      else { if (get_line_code(r) != atoi(val)) return false; }
    }
    // 4. nomeLinha
    else if (strcmp(name, "nomeLinha") == 0) {
      if (is_null) { if (get_line_name_size(r) > 0) return false; }
      else {
        char *line_name = get_line_name(r);
        if (line_name == NULL || strcmp(line_name, val) != 0) return false;
      }
    }
    // 5. codProxEstacao
    else if (strcmp(name, "codProxEstacao") == 0) {
      if (is_null) { if (get_next_station_code(r) != -1) return false; }
      else { if (get_next_station_code(r) != atoi(val)) return false; }
    }
    // 6. distProxEstacao
    else if (strcmp(name, "distProxEstacao") == 0) {
      if (is_null) { if (get_next_station_distance(r) != -1) return false; }
      else { if (get_next_station_distance(r) != atoi(val)) return false; }
    }
    // 7. codLinhaIntegra
    else if (strcmp(name, "codLinhaIntegra") == 0) {
      if (is_null) { if (get_integration_line_code(r) != -1) return false; }
      else { if (get_integration_line_code(r) != atoi(val)) return false; }
    }
    // 8. codEstIntegra
    else if (strcmp(name, "codEstIntegra") == 0) {
      if (is_null) { if (get_integration_station_code(r) != -1) return false; }
      else { if (get_integration_station_code(r) != atoi(val)) return false; }
    }
  }

  return true;
}

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

void select_where(char *bin_filename, int n) {
  //Abre o arquivo binário para leitura
  FILE *bin = fopen(bin_filename, "rb");
  if (!bin) {
    printf("Falha no processamento do arquivo.\n");
    return;
  }
  // Lê o header do arquivo binário para verificar seu status
  Header *header = new_header();
  read_header_binary(bin, header);
  // Verifica se o status do header é '0' (inconsistente). Se for, libera memória e fecha o arquivo.
  if (get_header_status(header) == '0') {
    printf("Falha no processamento do arquivo.\n");
    delete_header(&header);
    fclose(bin);
    return;
  }
  //loop para ler os critérios de busca, percorrer os registros e imprimir os que correspondem aos critérios
  for (int i = 0; i < n; i++) {
    int m;
    scanf("%d", &m);

    //Alocação de memória para os critérios de busca e leitura dos mesmos
    Criterion *criteria = malloc(m * sizeof(Criterion));
    for (int j = 0; j < m; j++) {
      scanf("%s", criteria[j].name);
      scan_quote_string(criteria[j].value);
    }
    //Posiciona o ponteiro do arquivo logo após o header para começar a leitura dos registros
    fseek(bin, 17, SEEK_SET);
    int found = 0;
    Record *record = new_record();
    //Loop para ler os registros do arquivo binário e verificar se correspondem aos critérios de busca usando a função match_record
    while (read_record_binary(bin, record)) {
      if (!is_removed(record) && match_record(record, criteria, m)) {
        print_record_one_line(record);
        found++;
      }
    }

    if (found == 0) {
      printf("Registro inexistente.\n");
    }

    delete_record(&record);
    free(criteria);
  }
  delete_header(&header);
  fclose(bin);
}

void select_by_rrn(char *bin_filename, int rrn) {
  FILE *bin = fopen(bin_filename, "rb");

  if (!bin) {
    printf("Falha no processamento do arquivo.\n");
    delete_string(&bin_filename);
    return;
  }

  fseek(bin, HEADER_SIZE + (long) rrn * RECORD_SIZE, SEEK_SET);

  Record *record = new_record();

  if (!read_record_binary(bin, record) || is_removed(record))
    printf("Registro inexistente.\n");
  else
    print_record_one_line(record);

  delete_record(&record);
  delete_string(&bin_filename);
  fclose(bin);
}
