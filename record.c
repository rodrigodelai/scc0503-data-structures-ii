#include "record.h"

// Reads an optional int field from CSV (empty field → -1), consuming the delimiter
static int read_optional_int_csv(FILE *csv, char delimiter) {
  int c = fgetc(csv);
  if (c == delimiter || c == '\n' || c == '\r' || c == EOF) {
    if (c == delimiter) ungetc(c, csv);
    return -1;
  }
  ungetc(c, csv);
  int val = -1;
  fscanf(csv, "%d", &val);
  return val;
}

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
  record->line_name_size           = 0;
  record->station_name             = NULL;
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

void read_record_binary(FILE *bin, Record *record) {
  if (fread(&record->removed, sizeof(char), 1, bin) != 1) return;
  fread(&record->next_removed_rrn,         sizeof(int), 1, bin);
  fread(&record->station_code,             sizeof(int), 1, bin);
  fread(&record->line_code,                sizeof(int), 1, bin);
  fread(&record->next_station_code,        sizeof(int), 1, bin);
  fread(&record->next_station_distance,    sizeof(int), 1, bin);
  fread(&record->integration_line_code,    sizeof(int), 1, bin);
  fread(&record->integration_station_code, sizeof(int), 1, bin);
  fread(&record->station_name_size,        sizeof(int), 1, bin);
  if (record->station_name_size > 0) {
    record->station_name = malloc(record->station_name_size + 1);
    fread(record->station_name, sizeof(char), record->station_name_size, bin);
    record->station_name[record->station_name_size] = '\0';
  }

  fread(&record->line_name_size,           sizeof(int), 1, bin);
  if (record->line_name_size > 0) {
    record->line_name = malloc(record->line_name_size + 1);
    fread(record->line_name, sizeof(char), record->line_name_size, bin);
    record->line_name[record->line_name_size] = '\0';
  }

  int remaining = NAMES_SIZE - record->station_name_size - (int)sizeof(int) - record->line_name_size;
  if (remaining > 0) fseek(bin, remaining, SEEK_CUR);

  return;
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
  written += sizeof(int);
  if (record->line_name_size > 0) {
    fwrite(record->line_name, sizeof(char), record->line_name_size, bin);
    written += record->line_name_size;
  }

  // Pad remaining bytes with trash
  int remaining = NAMES_SIZE - written;
  if (remaining > 0) {
    char trash[NAMES_SIZE];
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
  printf("%d,%s,%d,%s,%d,%d,%d,%d\n",
    record->station_code,
    record->station_name ? record->station_name : "",
    record->line_code,
    record->line_name ? record->line_name : "",
    record->next_station_code,
    record->next_station_distance,
    record->integration_line_code,
    record->integration_station_code);
}

void delete_record(Record **record) {
  if (!record || !*record) return;
  free((*record)->station_name);
  free((*record)->line_name);
  free(*record);
  *record = NULL;
}
