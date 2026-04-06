#include "header.h"

Header* new_header() {
  Header* header = (Header*) calloc(1, sizeof(Header));
  header->status = '0'; // arquivo inconsistente
  header->top_rrn = -1; // lista de removidos vazia
  header->stations_array = new_stations_array();
  header->pairs_array = new_pairs_array();
  return header;
}

void read_header_binary(FILE *binary, Header *header) {
  fseek(binary, 0, SEEK_SET);
  fread(&header->status, sizeof(char), 1, binary);
  fread(&header->top_rrn, sizeof(int), 1, binary);
  fread(&header->next_rrn, sizeof(int), 1, binary);
  fread(&header->stations, sizeof(int), 1, binary);
  fread(&header->pairs, sizeof(int), 1, binary);
}

void write_header_binary(FILE *binary, Header *header) {
  fseek(binary, 0, SEEK_SET);
  fwrite(&header->status, sizeof(char), 1, binary);
  fwrite(&header->top_rrn, sizeof(int), 1, binary);
  fwrite(&header->next_rrn, sizeof(int), 1, binary);
  fwrite(&header->stations, sizeof(int), 1, binary);
  fwrite(&header->pairs, sizeof(int), 1, binary);
}

void update_header_status_binary(FILE *binary, char status) {
  fseek(binary, 0, SEEK_SET);
  fwrite(&status, sizeof(char), 1, binary);
}

void update_header_control_fields(Header *header, Record *new_record) {
  if (include_station(header->stations_array, new_record->station_name))
    header->stations++;
  
  if (include_pair(header->pairs_array, new_record->station_code, new_record->next_station_code))
    header->pairs++;
  
  header->next_rrn++;
}

void print_header(Header *header) {
  printf("┌────────────────────────┐\n");
  printf("│      FILE HEADER       │\n");
  printf("├────────────────────────┤\n");
  printf("│ %-11s %10c │\n", "status",    header->status);
  printf("│ %-11s %10d │\n", "top_rrn",   header->top_rrn);
  printf("│ %-11s %10d │\n", "next_rrn",  header->next_rrn);
  printf("│ %-11s %10d │\n", "stations",  header->stations);
  printf("│ %-11s %10d │\n", "pairs",     header->pairs);
  printf("└────────────────────────┘\n");
}

void delete_header(Header **header) {
  if (!header || !*header) return;
  delete_stations_array(&(*header)->stations_array);
  delete_pairs_array(&(*header)->pairs_array);
  free(*header);
  *header = NULL;
}


// StationsArray Functions
StationsArray* new_stations_array() {
  StationsArray *stations = malloc(sizeof(StationsArray));
  stations->capacity = 64;
  stations->size = 0;
  stations->stations = calloc(64, sizeof(char*));
  return stations;
}

void populate_stations_array(StationsArray *stations, FILE *binary_file) {
  fseek(binary_file, HEADER_SIZE, SEEK_SET);

  Record *record = new_record();
  while (read_record_binary(binary_file, record)) {
    if (record->removed == '0' && record->station_name)
      include_station(stations, record->station_name);
  }
  delete_record(&record);
}

boolean include_station(StationsArray *stations, const char *name) {
  if (!name) return false;
  int i;
  for (i = 0; i < stations->size; i++)
    if (strcmp(stations->stations[i], name) == 0) return false; // already present
  if (i >= stations->capacity) {
    stations->capacity *= 2;
    stations->stations = realloc(stations->stations, stations->capacity * sizeof(char*));
    memset(stations->stations + i, 0, (stations->capacity - i) * sizeof(char*));
  }
  stations->stations[i] = strdup(name); // append at end
  stations->size++;
  return true;
}

int count_stations(StationsArray *stations) {
  return stations->size;
}

boolean is_station_present(StationsArray *stations, const char *name) {
  if (!name) return false;
  for (int i = 0; i < stations->size; i++)
    if (strcmp(stations->stations[i], name) == 0) return true;
  return false;
}

void delete_stations_array(StationsArray **stations) {
  if (!stations || !*stations) return;
  for (int i = 0; i < (*stations)->size; i++)
    free((*stations)->stations[i]);
  free((*stations)->stations);
  free(*stations);
  *stations = NULL;
}

// PairsArray Functions
PairsArray* new_pairs_array() {
  PairsArray *pairs = calloc(1, sizeof(PairsArray));
  return pairs;
}

boolean include_pair(PairsArray *pairs, int station_code, int next_station_code) {
  if (!pairs || station_code < 0 || station_code >= 1000 || next_station_code < 0 || next_station_code >= 1000) return false;
  if (is_pair_present(pairs, station_code, next_station_code)) return false;
  pairs->present[station_code][next_station_code] = true;
  return true;
}

int count_pairs(PairsArray *pairs) {
  if (!pairs) return 0;
  int count = 0;
  for (int i = 0; i < 1000; i++)
    for (int j = 0; j < 1000; j++)
      if (pairs->present[i][j]) count++;
  return count;
}

boolean is_pair_present(PairsArray *pairs, int station_code, int next_station_code) {
  if (!pairs || station_code < 0 || station_code >= 1000 || next_station_code < 0 || next_station_code >= 1000) return false;
  return pairs->present[station_code][next_station_code];
}

void delete_pairs_array(PairsArray **pairs) {
  if (!pairs || !*pairs) return;
  free(*pairs);
  *pairs = NULL;
}
