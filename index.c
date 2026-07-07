#include "index.h"

// Função de comparação para o qsort (ordena de forma crescente pelo codEstacao)
static int compare_index(const void *a, const void *b) {
    IndexEntry *entryA = (IndexEntry *)a;
    IndexEntry *entryB = (IndexEntry *)b;
    return entryA->codEstacao - entryB->codEstacao;
}

void create_index(char *bin_filename, char *index_filename) {
    // 1. Abre o arquivo de dados original para leitura
    FILE *bin = fopen(bin_filename, "rb");
    if (!bin) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Lê e verifica o status do cabeçalho do arquivo de dados
    Header *header = new_header();
    read_header_binary(bin, header);
    if (get_header_status(header) == '0') {
        printf("Falha no processamento do arquivo.\n");
        delete_header(&header);
        fclose(bin);
        return;
    }

    // Descobre o limite de registros lendo o tamanho do arquivo
    fseek(bin, 0, SEEK_END);
    long file_size = ftell(bin);
    int max_records = (file_size - HEADER_SIZE) / RECORD_SIZE;
    
    // Aloca vetor dinâmico para armazenar os índices em memória
    IndexEntry *entries = malloc(max_records * sizeof(IndexEntry));
    int count = 0;
    int current_rrn = 0;

    // Volta o ponteiro para o primeiro registro de dados
    fseek(bin, HEADER_SIZE, SEEK_SET);
    Record *record = new_record();

    // 2. Extrai codEstacao e RRN de todos os registros não removidos
    while (read_record_binary(bin, record)) {
        if (!is_removed(record)) {
            entries[count].codEstacao = get_station_code(record);
            entries[count].rrn = current_rrn;
            count++;
        }
        current_rrn++;
    }

    delete_record(&record);
    delete_header(&header);
    fclose(bin);

    // 3. Ordena o vetor de índices de forma crescente pelo codEstacao
    qsort(entries, count, sizeof(IndexEntry), compare_index);

    // 4. Cria e escreve o novo arquivo de índice
    FILE *idx = fopen(index_filename, "wb");
    if (!idx) {
        printf("Falha no processamento do arquivo.\n");
        free(entries);
        return;
    }

    // Escreve status '0' (inconsistente durante a escrita)
    char idx_status = '0';
    fwrite(&idx_status, sizeof(char), 1, idx);

    // Escreve os registros de índice ordenados
    for (int i = 0; i < count; i++) {
        fwrite(&entries[i].codEstacao, sizeof(int), 1, idx);
        fwrite(&entries[i].rrn, sizeof(int), 1, idx);
    }

    // Atualiza status para '1' (consistente após finalizar)
    idx_status = '1';
    fseek(idx, 0, SEEK_SET);
    fwrite(&idx_status, sizeof(char), 1, idx);
    fclose(idx);
    free(entries);

    // 5. Verifica a saída no terminal exigida pelas especificações
    binario_na_tela(index_filename);
}

int search_index_ram(IndexEntry *entries, int num_entries, int target_cod) {
    // Busca binária no vetor que já está em RAM. Toda a manipulação do índice
    // acontece em memória: nenhuma leitura de disco é feita aqui.
    int left = 0, right = num_entries - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (entries[mid].codEstacao == target_cod) {
            return entries[mid].rrn; // Achou!
        } else if (entries[mid].codEstacao < target_cod) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return -1; // Não encontrado
}

void set_index_status_binary(char *index_filename, char status) {
    // Abre em "rb+" para modificar apenas o byte de status sem apagar o resto
    // do arquivo (que ainda está em disco enquanto manipulamos a cópia em RAM).
    FILE *idx = fopen(index_filename, "rb+");
    if (!idx) return;

    fseek(idx, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, idx);
    fclose(idx);
}

IndexEntry* load_index(char *index_filename, int *num_entries) {
    FILE *idx = fopen(index_filename, "rb");
    if (!idx) {
        *num_entries = 0;
        return NULL;
    }

    fseek(idx, 0, SEEK_END);
    long size = ftell(idx);
    *num_entries = (size - 1) / 8; // Tira 1 byte do status, divide por 8 bytes cada registro

    if (*num_entries <= 0) {
        fclose(idx);
        return NULL;
    }

    IndexEntry *entries = malloc(*num_entries * sizeof(IndexEntry));
    fseek(idx, 1, SEEK_SET);
    
    // Lendo campo a campo para evitar problemas de padding do struct no compilador
    for (int i = 0; i < *num_entries; i++) {
        fread(&entries[i].codEstacao, sizeof(int), 1, idx);
        fread(&entries[i].rrn, sizeof(int), 1, idx);
    }

    fclose(idx);
    return entries;
}

void remove_from_index(IndexEntry *entries, int *num_entries, int target_cod) {
    for (int i = 0; i < *num_entries; i++) {
        if (entries[i].codEstacao == target_cod) {
            // Encontrou! Agora desloca todo mundo da direita para a esquerda
            for (int j = i; j < *num_entries - 1; j++) {
                entries[j] = entries[j + 1];
            }
            (*num_entries)--; // Diminui o tamanho do vetor
            break;
        }
    }
}

void rewrite_index(char *index_filename, IndexEntry *entries, int num_entries) {
    FILE *idx = fopen(index_filename, "wb"); // 'wb' recria o arquivo limpo
    if (!idx) return;

    char status = '0';
    fwrite(&status, sizeof(char), 1, idx);

    for (int i = 0; i < num_entries; i++) {
        fwrite(&entries[i].codEstacao, sizeof(int), 1, idx);
        fwrite(&entries[i].rrn, sizeof(int), 1, idx);
    }

    status = '1';
    fseek(idx, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, idx);
    fclose(idx);
}

void add_to_index(IndexEntry **entries, int *num_entries, int cod, int rrn) {
    // Aumenta o tamanho do vetor em 1 posição (realloc funciona como malloc se *entries for NULL)
    *entries = realloc(*entries, (*num_entries + 1) * sizeof(IndexEntry));
    
    // Adiciona o novo par no final
    (*entries)[*num_entries].codEstacao = cod;
    (*entries)[*num_entries].rrn = rrn;
    (*num_entries)++;
    
    // Usa a mesma função de comparação para manter o vetor ordenado
    qsort(*entries, *num_entries, sizeof(IndexEntry), compare_index);
}