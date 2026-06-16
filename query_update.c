#include "queries.h"
#include "index.h"

// Função auxiliar interna para realizar a remoção lógica na posição do RRN apontado
static void do_logical_removal(FILE *bin, Header *header, int rrn) {
    // 1. Vai até a posição exata do registro
    fseek(bin, HEADER_SIZE + rrn * RECORD_SIZE, SEEK_SET);
    
    // 2. Prepara os dados: novo status ('1') e pega o top_rrn antigo do cabeçalho
    char rem = '1';
    int next_rrn = get_header_top_rrn(header);
    
    // 3. Sobrescreve apenas os 5 primeiros bytes (1 do status + 4 do next_rrn)
    fwrite(&rem, sizeof(char), 1, bin);
    fwrite(&next_rrn, sizeof(int), 1, bin);
    
    // 4. Atualiza o cabeçalho na RAM para apontar para este novo "buraco" no topo da pilha
    set_header_top_rrn(header, rrn);
}

void delete_records(char *bin_filename, char *index_filename, int n) {
    // MUITO IMPORTANTE: Abrir com "r+b" para poder ler E escrever no mesmo arquivo sem apagá-lo
    FILE *bin = fopen(bin_filename, "r+b"); 
    if (!bin) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    Header *header = new_header();
    read_header_binary(bin, header);
    if (get_header_status(header) == '0') {
        printf("Falha no processamento do arquivo.\n");
        delete_header(&header);
        fclose(bin);
        return;
    }

    update_header_status_binary(bin, '0'); // Marca como inconsistente durante as operações

    // Carrega o índice para a RAM
    int num_entries = 0;
    IndexEntry *index_entries = load_index(index_filename, &num_entries);

    for (int i = 0; i < n; i++) {
        int m;
        scanf("%d", &m);
        Criterion *criteria = malloc(m * sizeof(Criterion));
        int target_codEstacao = -1;

        // Mesma leitura segura da Funcionalidade 6
        for (int j = 0; j < m; j++) {
            scanf("%s", criteria[j].name);
            if (strcmp(criteria[j].name, "nomeEstacao") == 0 || strcmp(criteria[j].name, "nomeLinha") == 0) {
                scan_quote_string(criteria[j].value);
            } else {
                scanf("%s", criteria[j].value);
                if (strcmp(criteria[j].value, "NULO") == 0) criteria[j].value[0] = '\0';
            }
            if (strcmp(criteria[j].name, "codEstacao") == 0 && criteria[j].value[0] != '\0') {
                target_codEstacao = atoi(criteria[j].value);
            }
        }

        Record *record = new_record();

        if (target_codEstacao != -1) {
            // ========= BUSCA INDEXADA =========
            int target_rrn = search_index(index_filename, target_codEstacao); // Pode usar a que já fizemos
            if (target_rrn != -1) {
                fseek(bin, HEADER_SIZE + target_rrn * RECORD_SIZE, SEEK_SET);
                if (read_record_binary(bin, record)) {
                    if (!is_removed(record) && match_record(record, criteria, m)) {
                        do_logical_removal(bin, header, target_rrn);
                        remove_from_index(index_entries, &num_entries, target_codEstacao);
                    }
                }
            }
        } else {
            // ========= BUSCA SEQUENCIAL =========
            fseek(bin, HEADER_SIZE, SEEK_SET);
            int current_rrn = 0;
            while (read_record_binary(bin, record)) {
                if (!is_removed(record) && match_record(record, criteria, m)) {
                    int cod = get_station_code(record);
                    
                    do_logical_removal(bin, header, current_rrn);
                    remove_from_index(index_entries, &num_entries, cod);

                    // Após o fseek do do_logical_removal, o ponteiro perdeu a posição de leitura.
                    // Precisamos reposicioná-lo para continuar lendo o próximo registro!
                    fseek(bin, HEADER_SIZE + (current_rrn + 1) * RECORD_SIZE, SEEK_SET);
                }
                current_rrn++;
            }
        }
        delete_record(&record);
        free(criteria);
    }

    // Grava o cabeçalho finalizado no arquivo
    write_header_binary(bin, header);
    update_header_status_binary(bin, '1'); // Volta a ficar consistente

    // Regrava o índice atualizado
    if (index_entries) {
        rewrite_index(index_filename, index_entries, num_entries);
        free(index_entries);
    }

    delete_header(&header);
    fclose(bin);

    // Saída exigida (Dados + Índice)
    binario_na_tela(bin_filename);
    binario_na_tela(index_filename);
}

void insert_records(char *bin_filename, char *index_filename, int n) {
    // "r+b" permite ler e escrever sem apagar o arquivo
    FILE *bin = fopen(bin_filename, "r+b");
    if (!bin) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    Header *header = new_header();
    read_header_binary(bin, header);
    if (get_header_status(header) == '0') {
        printf("Falha no processamento do arquivo.\n");
        delete_header(&header);
        fclose(bin);
        return;
    }
    
    update_header_status_binary(bin, '0'); // Status inconsistente durante a escrita

    // Carrega o índice para a RAM
    int num_entries = 0;
    IndexEntry *index_entries = load_index(index_filename, &num_entries);

    for (int i = 0; i < n; i++) {
        Record *new_rec = new_record();
        read_record_terminal(new_rec); // Lê os dados do novo registro da tela

        int target_rrn = get_header_top_rrn(header);
        
        if (target_rrn != -1) {
            // ========= REAPROVEITAMENTO DE ESPAÇO =========
            fseek(bin, HEADER_SIZE + target_rrn * RECORD_SIZE, SEEK_SET);
            
            // Lê o RRN do próximo buraco para atualizar o topo da pilha no cabeçalho
            char rem;
            int next_rem;
            fread(&rem, sizeof(char), 1, bin);
            fread(&next_rem, sizeof(int), 1, bin);
            
            set_header_top_rrn(header, next_rem);
            
            // Volta o ponteiro para o começo do espaço do registro para sobrescrever
            fseek(bin, HEADER_SIZE + target_rrn * RECORD_SIZE, SEEK_SET);
            
        } else {
            // ========= INSERÇÃO NO FINAL DO ARQUIVO =========
            target_rrn = get_header_next_rrn(header);
            fseek(bin, HEADER_SIZE + target_rrn * RECORD_SIZE, SEEK_SET);
            
            // Como usamos o final do arquivo, o next_rrn do arquivo cresce
            set_header_next_rrn(header, target_rrn + 1);
        }

        // Escreve o registro (a sua função write_record_binary já preenche a sobra com lixo '$')
        write_record_binary(bin, new_rec);
        
        // Adiciona a nova chave ao índice em RAM
        add_to_index(&index_entries, &num_entries, get_station_code(new_rec), target_rrn);
        
        delete_record(&new_rec);
    }

    // Grava o cabeçalho finalizado no arquivo e fecha status
    write_header_binary(bin, header);
    update_header_status_binary(bin, '1');

    // Regrava o arquivo de índice ordenado
    if (index_entries) {
        rewrite_index(index_filename, index_entries, num_entries);
        free(index_entries);
    }

    delete_header(&header);
    fclose(bin);

    // Saída exigida
    binario_na_tela(bin_filename);
    binario_na_tela(index_filename);
}