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
  Record *record = calloc(1, sizeof(Record));
  if (!record) {
    printf("Error: failed to allocate memory for record\n");
    return NULL;
  }
  record->removed                  = '0';
  record->next_removed_rrn         = -1;
  record->next_station_code        = -1;
  record->next_station_distance    = -1;
  record->integration_line_code    = -1;
  record->integration_station_code = -1;
  record->station_name_size        = 0;
  record->station_name             = NULL;
  record->line_name_size           = 0;
  record->line_name                = NULL;
  return record;
}

boolean read_record_csv(FILE *csv, Record *record) {
  // station_code
  if (fscanf(csv, "%d,", &record->station_code) != 1) {
    return false;
  }

  // station_name (up to comma)
  char buffer[44] = {0};
  if (fscanf(csv, "%43[^,\n],", buffer) != 1) {
    return false;
  }
  record->station_name_size = strlen(buffer);
  record->station_name ? free(record->station_name) : 0;
  record->station_name = strdup(buffer);

  // line_code
  if (fscanf(csv, "%d,", &record->line_code) != 1) {
    free(record->station_name); record->station_name = NULL;
    return false;
  }

  // line_name
  char buffer2[44] = {0};
  if (fscanf(csv, "%43[^,\n],", buffer2) != 1) {
    free(record->station_name); record->station_name = NULL;
    return false;
  }
  record->line_name_size = strlen(buffer2);
  record->line_name ? free(record->line_name) : 0;
  record->line_name = strdup(buffer2);

  // next_station_code (optional)
  record->next_station_code = read_optional_int_csv(csv, ',');
  { char c = fgetc(csv); if (c != ',') ungetc(c, csv); }

  // next_station_distance (optional)
  record->next_station_distance = read_optional_int_csv(csv, ',');
  { char c = fgetc(csv); if (c != ',') ungetc(c, csv); }

  // integration_line_code (optional)
  record->integration_line_code = read_optional_int_csv(csv, ',');

  // consume the comma after it if field was not empty
  char c = fgetc(csv);
  if (c != ',') ungetc(c, csv);

  // integration_station_code (optional, last field — ends with \n or EOF)
  record->integration_station_code = read_optional_int_csv(csv, '\n');

  return true;
}

boolean read_record_binary(FILE *bin, Record *record) {
  if (fread(&record->removed, sizeof(char), 1, bin) != 1) return false;
  fread(&record->next_removed_rrn,         sizeof(int), 1, bin);
  fread(&record->station_code,             sizeof(int), 1, bin);
  fread(&record->line_code,                sizeof(int), 1, bin);
  fread(&record->next_station_code,        sizeof(int), 1, bin);
  fread(&record->next_station_distance,    sizeof(int), 1, bin);
  fread(&record->integration_line_code,    sizeof(int), 1, bin);
  fread(&record->integration_station_code, sizeof(int), 1, bin);

  free(record->station_name); record->station_name = NULL;
  fread(&record->station_name_size,        sizeof(int), 1, bin);
  if (record->station_name_size > 0) {
    record->station_name = malloc(record->station_name_size + 1);
    fread(record->station_name, sizeof(char), record->station_name_size, bin);
    record->station_name[record->station_name_size] = '\0';
  }

  free(record->line_name); record->line_name = NULL;
  fread(&record->line_name_size,           sizeof(int), 1, bin);
  if (record->line_name_size > 0) {
    record->line_name = malloc(record->line_name_size + 1);
    fread(record->line_name, sizeof(char), record->line_name_size, bin);
    record->line_name[record->line_name_size] = '\0';
  }

  int remaining = NAME_SIZE - record->station_name_size - record->line_name_size;
  if (remaining > 0) fseek(bin, remaining, SEEK_CUR);

  return true;
}

int write_record_binary(FILE *bin, Record *record) {
  int rrn = (ftell(bin) - HEADER_SIZE) / RECORD_SIZE;

  fwrite(&record->removed,                 sizeof(char), 1, bin);
  fwrite(&record->next_removed_rrn,        sizeof(int),  1, bin);
  fwrite(&record->station_code,            sizeof(int),  1, bin);
  fwrite(&record->line_code,               sizeof(int),  1, bin);
  fwrite(&record->next_station_code,       sizeof(int),  1, bin);
  fwrite(&record->next_station_distance,   sizeof(int),  1, bin);
  fwrite(&record->integration_line_code,   sizeof(int),  1, bin);
  fwrite(&record->integration_station_code,sizeof(int),  1, bin);
  fwrite(&record->station_name_size,       sizeof(int),  1, bin);
  int written = 0;
  if (record->station_name_size > 0) {
    fwrite(record->station_name, sizeof(char), record->station_name_size, bin);
    written += record->station_name_size;
  }
  fwrite(&record->line_name_size,          sizeof(int),  1, bin);
  if (record->line_name_size > 0) {
    fwrite(record->line_name, sizeof(char), record->line_name_size, bin);
    written += record->line_name_size;
  }

  // Pad remaining bytes with trash
  int remaining = NAME_SIZE - written;
  if (remaining > 0) {
    char trash[NAME_SIZE];
    fill_with_trash(trash, remaining);
    fwrite(trash, sizeof(char), remaining, bin);
  }

  return rrn;
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

  if (record->line_code > 0)
    snprintf(line_code_str, sizeof(line_code_str), "%d", record->line_code);
  if (record->next_station_code > 0)
    snprintf(next_station_code_str, sizeof(next_station_code_str), "%d", record->next_station_code);
  if (record->next_station_distance > 0)
    snprintf(next_station_distance_str, sizeof(next_station_distance_str), "%d", record->next_station_distance);
  if (record->integration_line_code > 0)
    snprintf(integration_line_code_str, sizeof(integration_line_code_str), "%d", record->integration_line_code);
  if (record->integration_station_code > 0)
    snprintf(integration_station_code_str, sizeof(integration_station_code_str), "%d", record->integration_station_code);

  printf("%d %s %s %s %s %s %s %s\n",
    record->station_code,
    record->station_name,
    record->line_code > 0             ? line_code_str              : "NULO",
    record->line_name                 ? record->line_name           : "NULO",
    record->next_station_code > 0     ? next_station_code_str       : "NULO",
    record->next_station_distance > 0 ? next_station_distance_str   : "NULO",
    record->integration_line_code > 0 ? integration_line_code_str   : "NULO",
    record->integration_station_code > 0 ? integration_station_code_str : "NULO");
}

void delete_record(Record **record) {
  if (!record || !*record) return;
  free((*record)->station_name);
  free((*record)->line_name);
  free(*record);
  *record = NULL;
}

// Getters
boolean is_removed(Record *record)                 { return record->removed == REMOVED; }
int     get_next_removed_rrn(Record *record)       { return record->next_removed_rrn; }
int     get_station_code(Record *record)           { return record->station_code; }
int     get_line_code(Record *record)              { return record->line_code; }
int     get_next_station_code(Record *record)      { return record->next_station_code; }
int     get_next_station_distance(Record *record)  { return record->next_station_distance; }
int     get_integration_line_code(Record *record)  { return record->integration_line_code; }
int     get_integration_station_code(Record *record){ return record->integration_station_code; }
int     get_station_name_size(Record *record)      { return record->station_name_size; }
char*   get_station_name(Record *record)           { return record->station_name; }
int     get_line_name_size(Record *record)         { return record->line_name_size; }
char*   get_line_name(Record *record)              { return record->line_name; }

// Setters
boolean set_removed(Record *record, Removed removed) {
  if (removed != REMOVED && removed != NOT_REMOVED) return false;
  record->removed = removed;
  return true;
}

boolean set_next_removed_rrn(Record *record, int next_removed_rrn) {
  if (next_removed_rrn < -1) return false;
  record->next_removed_rrn = next_removed_rrn;
  return true;
}

boolean set_station_code(Record *record, int station_code) {
  if (station_code < 0) return false;
  record->station_code = station_code;
  return true;
}

boolean set_line_code(Record *record, int line_code) {
  if (line_code < -1) return false;
  record->line_code = line_code;
  return true;
}

boolean set_next_station_code(Record *record, int next_station_code) {
  if (next_station_code < -1) return false;
  record->next_station_code = next_station_code;
  return true;
}

boolean set_next_station_distance(Record *record, int next_station_distance) {
  if (next_station_distance < -1) return false;
  record->next_station_distance = next_station_distance;
  return true;
}

boolean set_integration_line_code(Record *record, int integration_line_code) {
  if (integration_line_code < -1) return false;
  record->integration_line_code = integration_line_code;
  return true;
}

boolean set_integration_station_code(Record *record, int integration_station_code) {
  if (integration_station_code < -1) return false;
  record->integration_station_code = integration_station_code;
  return true;
}

boolean set_station_name_size(Record *record, int station_name_size) {
  if (station_name_size <= 0) return false;
  record->station_name_size = station_name_size;
  return true;
}

boolean set_station_name(Record *record, char *station_name) {
  if (!station_name) return false;
  free(record->station_name);
  record->station_name = strdup(station_name);
  record->station_name_size = strlen(record->station_name);
  return true;
}

boolean set_line_name_size(Record *record, int line_name_size) {
  if (line_name_size < 0) return false;
  record->line_name_size = line_name_size;
  return true;
}

boolean set_line_name(Record *record, char *line_name) {
  free(record->line_name);
  record->line_name = line_name ? strdup(line_name) : NULL;
  record->line_name_size = record->line_name ? strlen(record->line_name) : 0;
  return true;
}
