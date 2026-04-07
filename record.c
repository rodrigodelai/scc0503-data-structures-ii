#include "record.h"

struct record_st {
  char removed;                 // 1 byte: '1' removido, '0' não removido
  int next_removed_rrn;         // 4 bytes: RRN do próximo removido ou -1
  int station_code;             // 4 bytes: código da estacao
  int line_code;                // 4 bytes: código da linha
  int next_station_code;        // 4 bytes: código da proxima estacao
  int next_station_distance;    // 4 bytes: distancia ate a proxima estacao
  int integration_line_code;    // 4 bytes: codigo da linha de integracao
  int integration_station_code; // 4 bytes: codigo da estacao de integracao
  int station_name_size;        // 4 bytes: tamanho do nome da estacao
  char *station_name;           // N bytes: nome da estacao (max 43 bytes)
  int line_name_size;           // 4 bytes: tamanho do nome da linha
  char *line_name;              // M bytes: nome da linha; N + M = 43 bytes
};

Record* new_record() {
  Record *record = calloc(1, sizeof(Record)); // aloca memoria e inicializa com zeros

  if (!record) {
    printf("Error: failed to allocate memory for record\n");
    return NULL;
  }

  record->removed                  = '0'; // nao removido
  record->next_removed_rrn         = -1; // valor invalido (campo vazio)
  record->next_station_code        = -1; // valor invalido (campo vazio)
  record->next_station_distance    = -1; // valor invalido (campo vazio)
  record->integration_line_code    = -1; // valor invalido (campo vazio)
  record->integration_station_code = -1; // valor invalido (campo vazio)

  return record;
}

boolean read_record_csv(FILE *csv, Record *record) {
  // leitura do codigo da estacao
  if (fscanf(csv, "%d,", &record->station_code) != 1) { // tenta ler o codigo da estacao e verifica se leu algo
    return false; // retorna falso em caso de erro
  }

  // leitura do nome da estacao (ate a virgula)
  char buffer[NAME_SIZE + 1] = {0};
  if (fscanf(csv, "%43[^,\n],", buffer) != 1) { // tenta ler o nome da estacao e verifica se leu algo
    return false; // retorna falso em caso de erro
  }

  record->station_name_size = strlen(buffer); // tamanho do nome da estacao
  record->station_name ? free(record->station_name) : 0; // libera memoria se ja houver algo
  record->station_name = strdup(buffer); // copia o nome da estacao

  // leitura do codigo da linha (campo opcional)
  record->line_code = read_optional_int_csv(csv, ',');

  // leitura do nome da linha (campo opcional)
  free(record->line_name); // libera memoria se ja houver algo
  record->line_name = read_optional_string_csv(csv, ',');
  record->line_name_size = record->line_name ? strlen(record->line_name) : 0; // tamanho do nome da linha

  // leitura do codigo da proxima estacao (campo opcional)
  record->next_station_code = read_optional_int_csv(csv, ',');

  // leitura da distancia da proxima estacao (campo opcional)
  record->next_station_distance = read_optional_int_csv(csv, ',');

  // leitura do codigo da linha de integracao (campo opcional)
  record->integration_line_code = read_optional_int_csv(csv, ',');

  // leitura do codigo da estacao de integracao (campo opcional, ultimo campo)
  record->integration_station_code = read_optional_int_csv(csv, '\n');

  return true;
}

boolean read_record_binary(FILE *bin, Record *record) {
  if (fread(&record->removed, sizeof(char), 1, bin) != 1) // le o campo removido
    return false; // retorna falso em caso de erro

  fread(&record->next_removed_rrn,         sizeof(int), 1, bin); // | 
  fread(&record->station_code,             sizeof(int), 1, bin); // |
  fread(&record->line_code,                sizeof(int), 1, bin); // |
  fread(&record->next_station_code,        sizeof(int), 1, bin); // | leitura campo a campo
  fread(&record->next_station_distance,    sizeof(int), 1, bin); // | 
  fread(&record->integration_line_code,    sizeof(int), 1, bin); // |
  fread(&record->integration_station_code, sizeof(int), 1, bin); // |
  
  fread(&record->station_name_size,        sizeof(int), 1, bin);
  free(record->station_name); // libera memoria se ja houver algo
  record->station_name = NULL; // inicializa como NULL

  if (record->station_name_size > 0) { // se o tamanho for maior que 0, aloca memoria
    record->station_name = malloc(record->station_name_size + 1); // +1 byte para o caractere nulo
    fread(record->station_name, sizeof(char), record->station_name_size, bin); // le o nome da estacao
    record->station_name[record->station_name_size] = '\0'; // adiciona o caractere nulo ao final
  }

  fread(&record->line_name_size,           sizeof(int), 1, bin); // le o tamanho do nome da linha
  free(record->line_name); // libera memoria se ja houver algo
  record->line_name = NULL; // inicializa como NULL

  if (record->line_name_size > 0) { // se o tamanho for maior que 0, aloca memoria
    record->line_name = malloc(record->line_name_size + 1); // +1 byte para o caractere nulo
    fread(record->line_name, sizeof(char), record->line_name_size, bin); // le o nome da linha
    record->line_name[record->line_name_size] = '\0'; // adiciona o caractere nulo ao final
  }

  int remaining = NAME_SIZE - record->station_name_size - record->line_name_size; // calcula os bytes restantes
  if (remaining > 0) fseek(bin, remaining, SEEK_CUR); // pula os bytes restantes

  return true; // leu com sucesso
}

int write_record_binary(FILE *bin, Record *record) {
  int rrn = (ftell(bin) - HEADER_SIZE) / RECORD_SIZE; // calcula o rrn

  fwrite(&record->removed,                 sizeof(char), 1, bin); // |
  fwrite(&record->next_removed_rrn,        sizeof(int),  1, bin); // |
  fwrite(&record->station_code,            sizeof(int),  1, bin); // |
  fwrite(&record->line_code,               sizeof(int),  1, bin); // |
  fwrite(&record->next_station_code,       sizeof(int),  1, bin); // | escrita campo a campo
  fwrite(&record->next_station_distance,   sizeof(int),  1, bin); // |
  fwrite(&record->integration_line_code,   sizeof(int),  1, bin); // |
  fwrite(&record->integration_station_code,sizeof(int),  1, bin); // |
  fwrite(&record->station_name_size,       sizeof(int),  1, bin); // |

  int written = 0; // contador de bytes escritos
  
  if (record->station_name_size > 0) {
    fwrite(record->station_name, sizeof(char), record->station_name_size, bin);
    written += record->station_name_size; // atualiza o contador
  }

  fwrite(&record->line_name_size,          sizeof(int),  1, bin);
  if (record->line_name_size > 0) {
    fwrite(record->line_name, sizeof(char), record->line_name_size, bin);
    written += record->line_name_size; // atualiza o contador
  }

  // preenche os bytes restantes com marcador de lixo ($)
  int remaining = NAME_SIZE - written;

  if (remaining > 0) {
    char trash[NAME_SIZE];
    fill_with_trash(trash, remaining);
    fwrite(trash, sizeof(char), remaining, bin);
  }

  return rrn; // retorna o rrn do registro escrito
}

void print_record(Record *reg) {
  printf("┌─────────────────────────────────────────┐\n");
  printf("│             STATION RECORD              │\n");
  printf("├─────────────────────────────────────────┤\n");
  printf("│ %-24s %14c │\n", "removed",                  reg->removed);
  printf("│ %-24s %14d │\n", "next_removed_rrn",         reg->next_removed_rrn);
  printf("│ %-24s %14d │\n", "station_code",             reg->station_code);
  printf("│ %-24s %14d │\n", "line_code",                reg->line_code);
  printf("│ %-24s %14d │\n", "next_station_code",        reg->next_station_code);
  printf("│ %-24s %14d │\n", "next_station_distance",    reg->next_station_distance);
  printf("│ %-24s %14d │\n", "integration_line_code",    reg->integration_line_code);
  printf("│ %-24s %14d │\n", "integration_station_code", reg->integration_station_code);
  printf("│ %-24s %14d │\n", "station_name_size",        reg->station_name_size);
  printf("│ %-24s %14s │\n", "station_name",             reg->station_name ? reg->station_name : "");
  printf("│ %-24s %14d │\n", "line_name_size",           reg->line_name_size);
  printf("│ %-24s %14s │\n", "line_name",                reg->line_name ? reg->line_name : "");
  printf("└─────────────────────────────────────────┘\n");
}

void print_record_one_line(Record *record) {
  char line_code_str[12], next_station_code_str[12],
       next_station_distance_str[12], integration_line_code_str[12],
       integration_station_code_str[12];

  if (record->line_code > 0) // se o codigo da linha for maior que 0, converte para string
    snprintf(line_code_str, sizeof(line_code_str), "%d", record->line_code);
  if (record->next_station_code > 0) // se o codigo da proxima estacao for maior que 0, converte para string
    snprintf(next_station_code_str, sizeof(next_station_code_str), "%d", record->next_station_code);
  if (record->next_station_distance > 0) // se a distancia para a proxima estacao for maior que 0, converte para string
    snprintf(next_station_distance_str, sizeof(next_station_distance_str), "%d", record->next_station_distance);
  if (record->integration_line_code > 0) // se o codigo da linha de integracao for maior que 0, converte para string
    snprintf(integration_line_code_str, sizeof(integration_line_code_str), "%d", record->integration_line_code);
  if (record->integration_station_code > 0) // se o codigo da estacao de integracao for maior que 0, converte para string
    snprintf(integration_station_code_str, sizeof(integration_station_code_str), "%d", record->integration_station_code);

  // imprime o registro em uma linha; exibe "NULO" se valor do campo for invalido (campo vazio)
  printf("%d %s %s %s %s %s %s %s\n",
    record->station_code,
    record->station_name,
    record->line_code > 0                ? line_code_str                : "NULO",
    record->line_name                    ? record->line_name            : "NULO",
    record->next_station_code > 0        ? next_station_code_str        : "NULO",
    record->next_station_distance > 0    ? next_station_distance_str    : "NULO",
    record->integration_line_code > 0    ? integration_line_code_str    : "NULO",
    record->integration_station_code > 0 ? integration_station_code_str : "NULO");
}

void delete_record(Record **record) {
  if (!record || !*record) return; // protecao contra ponteiro nulo
  free((*record)->station_name); // libera memoria alocada para o nome da estacao
  free((*record)->line_name); // libera memoria alocada para o nome da linha
  free(*record); // libera memoria alocada para o ponteiro
  *record = NULL; // protecao contra dangling pointer
}

// Getters
boolean is_removed(Record *record)            { return record->removed == '1'; }
int     get_station_code(Record *record)      { return record->station_code; }
int     get_next_station_code(Record *record) { return record->next_station_code; }
char*   get_station_name(Record *record)      { return record->station_name; }
