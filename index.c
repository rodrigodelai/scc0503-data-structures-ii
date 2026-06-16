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