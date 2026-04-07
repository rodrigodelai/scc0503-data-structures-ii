#include "header.h"
#include <stdint.h>

// Declaracoes de estruturas internas ────────────────────────────────────────────────────────

#define INITIAL_ARRAY_CAPACITY 64

/**
 * Estrutura interna para armazenar nome das estacoes e verificar unicidade.
 */
typedef struct {
  int capacity;
  int size;
  char** stations;
} StationsArray;

/**
 * Cria e inicializa um novo array de estacoes.
 * @return Ponteiro para o novo array de estacoes.
 */
static StationsArray* new_stations_array();

/**
 * Adiciona uma estacao ao array.
 * @param stations Ponteiro para o array de estacoes.
 * @param name Nome da estacao.
 * @return true se adicionou, false se ja existia.
 */
static boolean add_station(StationsArray *stations, char *name);

/**
 * Busca binaria de uma estacao no array.
 * @param stations Ponteiro para o array de estacoes.
 * @param name Nome da estacao.
 * @return Indice se encontrou, "-(indice_de_insercao)" se nao encontrou.
 */
static int search_station(StationsArray *stations, char *name);

/**
 * Libera memoria do array de estacoes.
 * @param stations Ponteiro para o ponteiro do array de estacoes.
 */
static void delete_stations_array(StationsArray **stations);



/**
 * Estrutura interna para armazenar pares de estacoes e verificar unicidade.
 */
typedef struct {
  uint32_t present[1000][32]; // 1000 estacoes, 32 palavras de 32 bits cada
} PairsArray;

/**
 * Cria e inicializa um novo array de pares de estacoes.
 * @return Ponteiro para o novo array de pares de estacoes.
 */
static PairsArray* new_pairs_array();

/**
 * Adiciona um par de estacoes ao array.
 * @param pairs Ponteiro para o array de pares de estacoes.
 * @param station_code Codigo da estacao.
 * @param next_station_code Codigo da proxima estacao.
 * @return true se adicionou, false se ja existia.
 */
static boolean add_pair(PairsArray *pairs, int station_code, int next_station_code);

/**
 * Verifica se um par de estacoes esta no array.
 * @param pairs Ponteiro para o array de pares de estacoes.
 * @param station_code Codigo da estacao.
 * @param next_station_code Codigo da proxima estacao.
 * @return true se esta presente, false caso contrario.
 */
static boolean is_pair_present(PairsArray *pairs, int station_code, int next_station_code);

/**
 * Libera memoria do array de pares de estacoes.
 * @param pairs Ponteiro para o ponteiro do array de pares de estacoes.
 */
static void delete_pairs_array(PairsArray **pairs);



// Implementacao de funcoes expostas no cabecalho ─────────────────────────────

struct header_st {
  char status;           // 1 byte: '0' inconsistente, '1' consistente
  int top_rrn;           // 4 bytes: RRN do ultimo registro removido
  int next_rrn;          // 4 bytes: proximo RRN disponivel
  int stations;          // 4 bytes: quantidade de estacoes
  int pairs;             // 4 bytes: quantidade de pares

  StationsArray* stations_array; // identifica estacoes unicas
  PairsArray* pairs_array;       // identifica pares de estacoes
};

Header* new_header() {
  Header* header = (Header*) calloc(1, sizeof(Header)); // inicializa campos com 0
  header->status = '0'; // marca como arquivo inconsistente
  header->top_rrn = -1; // indica que a lista de removidos esta vazia
  header->stations_array = new_stations_array(); // inicializa array de estacoes
  header->pairs_array = new_pairs_array(); // inicializa array de pares
  return header;
}

void read_header_binary(FILE *binary, Header *header) {
  fseek(binary, 0, SEEK_SET); // aponta para inicio do arquivo
  fread(&header->status, sizeof(char), 1, binary);   // |
  fread(&header->top_rrn, sizeof(int), 1, binary);   // |
  fread(&header->next_rrn, sizeof(int), 1, binary);  // | leitura campo a campo
  fread(&header->stations, sizeof(int), 1, binary);  // |
  fread(&header->pairs, sizeof(int), 1, binary);     // |
}

void write_header_binary(FILE *binary, Header *header) {
  fseek(binary, 0, SEEK_SET); // aponta para inicio do arquivo
  fwrite(&header->status, sizeof(char), 1, binary);  // |
  fwrite(&header->top_rrn, sizeof(int), 1, binary);  // |
  fwrite(&header->next_rrn, sizeof(int), 1, binary); // | escrita campo a campo
  fwrite(&header->stations, sizeof(int), 1, binary); // |
  fwrite(&header->pairs, sizeof(int), 1, binary);    // |
}

void update_header_status_binary(FILE *binary, char status) {
  fseek(binary, 0, SEEK_SET); // aponta para inicio do arquivo
  fwrite(&status, sizeof(char), 1, binary);
}

void update_header_control_fields(Header *header, Record *new_record) {
  if (add_station(header->stations_array, get_station_name(new_record))) // se estacao nao existia
    header->stations++;

  if (add_pair(header->pairs_array, get_station_code(new_record), get_next_station_code(new_record))) // se par nao existia
    header->pairs++;

  header->next_rrn++; // atualiza proximo RRN disponivel
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
  if (!header || !*header) return; // protecao contra null pointer
  delete_stations_array(&(*header)->stations_array); // libera memoria do array de estacoes
  delete_pairs_array(&(*header)->pairs_array); // libera memoria do array de pares
  free(*header); // libera memoria do header
  *header = NULL; // evita dangling pointer
}

char get_header_status(Header *header) {
    if (header == NULL) return '0'; // protecao contra null pointer
    return header->status;
}



// Implementacao de estruturas internas ───────────────────────────────────────

static StationsArray* new_stations_array() {
  StationsArray *stations = malloc(sizeof(StationsArray)); // aloca memoria para o array de estacoes
  stations->capacity = INITIAL_ARRAY_CAPACITY;
  stations->size = 0;
  stations->stations = calloc(INITIAL_ARRAY_CAPACITY, sizeof(char*)); // inicializa com NULLs
  return stations;
}

static boolean add_station(StationsArray *stations, char *name) {
  if (!name) return false; // protecao contra null pointer

  int index = search_station(stations, name); // busca binaria; retorno negativo contem indice de insercao
  if (index >= 0) return false; // estacao ja existe
  
  int position = -(index + 1); // ponto de insercao para manter ordenado

  if (stations->size >= stations->capacity) { // redimensiona se necessario
    stations->capacity *= 2; // dobra a capacidade
    stations->stations = realloc(stations->stations, stations->capacity * sizeof(char*)); // realoca memoria
    memset(stations->stations + stations->size, 0, (stations->capacity - stations->size) * sizeof(char*)); // preenche com NULLs
  }

  memmove(stations->stations + position + 1, stations->stations + position, (stations->size - position) * sizeof(char*)); // desloca elementos
  stations->stations[position] = strdup(name); // copia nome da estacao
  stations->size++; // incrementa tamanho do array
  return true;
}

static int search_station(StationsArray *stations, char *name) { // busca binaria
  int lower = 0, higher = stations->size - 1;

  while (lower <= higher) {
    int mid = lower + (higher - lower) / 2;
    int comparation = strcmp(stations->stations[mid], name);
    if (comparation == 0) return mid;
    if (comparation < 0)  lower = mid + 1;
    else                  higher = mid - 1;
  }

  return -(lower + 1); // retorno negativo com ponto de insercao para manter ordenado
}

static void delete_stations_array(StationsArray **stations) {
  if (!stations || !*stations) return; // protecao contra null pointer

  for (int i = 0; i < (*stations)->size; i++) // libera memoria de cada string
    free((*stations)->stations[i]);

  free((*stations)->stations); // libera memoria do array de ponteiros
  free(*stations); // libera memoria do struct
  *stations = NULL; // evita dangling pointer
}

static PairsArray* new_pairs_array() {
  PairsArray *pairs = calloc(1, sizeof(PairsArray)); // aloca memoria e inicializa struct com zeros
  return pairs;
}

static boolean add_pair(PairsArray *pairs, int station_code, int next_station_code) {
  if (!pairs || station_code < 0 || station_code >= 1000 ||
      next_station_code < 0 || next_station_code >= 1000) 
      return false; // protecao contra valores invalidos

  if (is_pair_present(pairs, station_code, next_station_code)) return false; // ja existe (nao adicionado)
  pairs->present[station_code][next_station_code >> 5] |= 1u << (next_station_code & 31); // marca par como presente
  return true; // sucesso (par adicionado)
}

static boolean is_pair_present(PairsArray *pairs, int station_code, int next_station_code) {
  if (!pairs || station_code < 0 || station_code >= 1000 || 
      next_station_code < 0 || next_station_code >= 1000)
      return false; // protecao contra valores invalidos
  
  return (pairs->present[station_code][next_station_code >> 5] >> (next_station_code & 31)) & 1; // verifica se bit esta setado (par presente)
}

static void delete_pairs_array(PairsArray **pairs) {
  if (!pairs || !*pairs) return; // protecao contra null pointer
  free(*pairs); // libera memoria da struct
  *pairs = NULL; // evita dangling pointer
}
