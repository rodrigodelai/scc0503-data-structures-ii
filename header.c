#include "header.h"

Header* new_header() {
  Header* header = (Header*) calloc(1, sizeof(Header));
  header->status = '0'; // arquivo inconsistente
  header->top_rrn = -1; // lista de removidos vazia
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

void update_header_status(FILE *binary, char status) {
  fseek(binary, 0, SEEK_SET);
  fwrite(&status, sizeof(char), 1, binary);
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
  free(*header);
  *header = NULL;
}
