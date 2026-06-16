#include "queries.h"

// Declaracoes de estruturas internas ─────────────────────────────────────────

/**
 * Estrutura para armazenar critérios de busca.
 */
typedef struct {
  char name[50];
  char value[100];
} Criterion;

/**
 * Verifica se um registro atende aos critérios de busca.
 * @param r Ponteiro para o registro a ser verificado.
 * @param criteria Ponteiro para os critérios de busca.
 * @param m Numero de critérios de busca.
 * @return true se o registro atende aos critérios, false caso contrário.
 */
static boolean match_record(Record *r, Criterion *criteria, int m);

// Implementacao de funcoes expostas no cabecalho ─────────────────────────────

void select_all(char *bin_filename) {
  // open file
  FILE *bin = fopen(bin_filename, "rb");

  if (!bin) {
    printf("Falha no processamento do arquivo.\n");
    delete_string(&bin_filename);
    return;
  }

  // read header
  Header *header = new_header();
  read_header_binary(bin, header);

  // VERIFICAÇÃO DO STATUS DO CABEÇALHO (Correção do Seg Fault)
  if (get_header_status(header) == '0') {
    printf("Falha no processamento do arquivo.\n");
    delete_header(&header);
    delete_string(&bin_filename);
    fclose(bin);
    return;
  }

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
  
  // Verifica se o status do header é '0' (inconsistente).
  if (get_header_status(header) == '0') {
    printf("Falha no processamento do arquivo.\n");
    delete_header(&header);
    fclose(bin);
    return;
  }
  
  //loop para ler os critérios de busca
  for (int i = 0; i < n; i++) {
    int m;
    scanf("%d", &m);

    //Alocação de memória para os critérios de busca
    Criterion *criteria = malloc(m * sizeof(Criterion));
    boolean search_by_unique_key = false;

    for (int j = 0; j < m; j++) {
      scanf("%s", criteria[j].name);

      // Checa se o campo buscado é do tipo string
      if (strcmp(criteria[j].name, "nomeEstacao") == 0 || 
          strcmp(criteria[j].name, "nomeLinha") == 0) {
          
        scan_quote_string(criteria[j].value);
        
      } else {
        // Se for um campo numérico, lemos com scanf normal (evita o bug do dígito único)
        scanf("%s", criteria[j].value);
        
        // Se o usuário digitou NULO para um inteiro, convertemos para string vazia 
        // para que a lógica do match_record funcione perfeitamente.
        if (strcmp(criteria[j].value, "NULO") == 0) {
          criteria[j].value[0] = '\0';
        }
      }

      // Verifica se a busca envolve a chave primária
      if (strcmp(criteria[j].name, "codEstacao") == 0) {
        search_by_unique_key = true;
      }
    }
    
    fseek(bin, 17, SEEK_SET);
    int found = 0;
    Record *record = new_record();
    
    while (read_record_binary(bin, record)) {
      if (!is_removed(record) && match_record(record, criteria, m)) {
        print_record_one_line(record);
        found++;

        // INTERROMPE A BUSCA NA CHAVE PRIMÁRIA (Otimização)
        if (search_by_unique_key) {
            break; 
        }
      }
    }

    if (found == 0) {
      printf("Registro inexistente.\n");
    }

    delete_record(&record);
    free(criteria);
    if (i < n - 1) printf("\n");
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

  // CHECAR STATUS DO HEADER (Correção do Seg Fault)
  char status;
  fread(&status, sizeof(char), 1, bin);
  if (status == '0') {
    printf("Falha no processamento do arquivo.\n");
    delete_string(&bin_filename);
    fclose(bin);
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

// Implementacao de funcoes internas ──────────────────────────────────────────

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