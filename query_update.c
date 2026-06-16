#include "queries.h"
#include "index.h"

// Recalcula os contadores de controle do cabeçalho (estações únicas e pares
// válidos) varrendo os registros ativos do arquivo de dados.
// - stations: quantidade de nomes de estação distintos entre registros ativos.
// - pairs: quantidade de registros ativos cujo par (codEstacao, codProxEstacao)
//   é válido, ou seja, ambos os códigos estão no intervalo [0, 1000) (não nulos).
static void compute_active_counters(FILE *bin, int *out_stations, int *out_pairs) {
    fseek(bin, HEADER_SIZE, SEEK_SET);
    Record *rec = new_record();
    char **names = NULL;
    int n_names = 0;
    int pairs = 0;

    while (read_record_binary(bin, rec)) {
        if (is_removed(rec)) continue;

        int cod = get_station_code(rec);
        int nsc = get_next_station_code(rec);
        if (cod >= 0 && cod < 1000 && nsc >= 0 && nsc < 1000) pairs++;

        char *name = get_station_name(rec);
        if (name) {
            boolean found = false;
            for (int k = 0; k < n_names; k++)
                if (strcmp(names[k], name) == 0) { found = true; break; }
            if (!found) {
                names = realloc(names, (n_names + 1) * sizeof(char *));
                names[n_names++] = strdup(name);
            }
        }
    }

    for (int k = 0; k < n_names; k++) free(names[k]);
    free(names);
    delete_record(&rec);

    *out_stations = n_names;
    *out_pairs = pairs;
}

// Reescreve os contadores de controle do cabeçalho preservando o "drift"
// (diferença entre o valor armazenado e o recalculado no momento da abertura).
// Isso reproduz fielmente a manutenção incremental dos contadores, inclusive
// quando o arquivo de entrada já possui um contador inconsistente com os dados.
static void fix_header_counters(FILE *bin, Header *header, int drift_stations, int drift_pairs) {
    int rec_st, rec_pr;
    compute_active_counters(bin, &rec_st, &rec_pr);
    set_header_stations(header, rec_st + drift_stations);
    set_header_pairs(header, rec_pr + drift_pairs);
}

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

    // Calcula o drift dos contadores antes de qualquer modificação
    int drift_st, drift_pr;
    {
        int rec_st, rec_pr;
        compute_active_counters(bin, &rec_st, &rec_pr);
        drift_st = get_header_stations(header) - rec_st;
        drift_pr = get_header_pairs(header) - rec_pr;
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

    // Atualiza os contadores de controle do cabeçalho frente às remoções
    fix_header_counters(bin, header, drift_st, drift_pr);

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
    
    // Calcula o drift dos contadores antes de qualquer modificação
    int drift_st, drift_pr;
    {
        int rec_st, rec_pr;
        compute_active_counters(bin, &rec_st, &rec_pr);
        drift_st = get_header_stations(header) - rec_st;
        drift_pr = get_header_pairs(header) - rec_pr;
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

    // Atualiza os contadores de controle do cabeçalho frente às inserções
    fix_header_counters(bin, header, drift_st, drift_pr);

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

// Função auxiliar que aplica os novos valores da busca em um Registro que está na RAM
static void apply_updates(Record *r, Criterion *updates, int p, int *old_cod, int *new_cod) {
  *old_cod = get_station_code(r);
  *new_cod = *old_cod; // Assume que não mudou inicialmente

  for (int i = 0; i < p; i++) {
    char *name = updates[i].name;
    char *val = updates[i].value;
    boolean is_null = (val[0] == '\0');
    int val_int = is_null ? -1 : atoi(val);

    if (strcmp(name, "codEstacao") == 0) {
      set_station_code(r, val_int);
      *new_cod = val_int; // Captura caso a chave primária mude
    } else if (strcmp(name, "nomeEstacao") == 0) {
      set_station_name(r, is_null ? NULL : val);
    } else if (strcmp(name, "codLinha") == 0) {
      set_line_code(r, val_int);
    } else if (strcmp(name, "nomeLinha") == 0) {
      set_line_name(r, is_null ? NULL : val);
    } else if (strcmp(name, "codProxEstacao") == 0) {
      set_next_station_code(r, val_int);
    } else if (strcmp(name, "distProxEstacao") == 0) {
      set_next_station_distance(r, val_int);
    } else if (strcmp(name, "codLinhaIntegra") == 0) {
      set_integration_line_code(r, val_int);
    } else if (strcmp(name, "codEstIntegra") == 0) {
      set_integration_station_code(r, val_int);
    }
  }
}

void update_records(char *bin_filename, char *index_filename, int n) {
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
    
    // Calcula o drift dos contadores antes de qualquer modificação
    int drift_st, drift_pr;
    {
        int rec_st, rec_pr;
        compute_active_counters(bin, &rec_st, &rec_pr);
        drift_st = get_header_stations(header) - rec_st;
        drift_pr = get_header_pairs(header) - rec_pr;
    }

    update_header_status_binary(bin, '0'); // Status inconsistente

    int num_entries = 0;
    IndexEntry *index_entries = load_index(index_filename, &num_entries);

    for (int i = 0; i < n; i++) {
        // --- 1. LÊ CRITÉRIOS DE BUSCA (m) ---
        int m; scanf("%d", &m);
        Criterion *search_criteria = malloc(m * sizeof(Criterion));
        int target_codEstacao = -1;

        for (int j = 0; j < m; j++) {
            scanf("%s", search_criteria[j].name);
            if (strcmp(search_criteria[j].name, "nomeEstacao") == 0 || strcmp(search_criteria[j].name, "nomeLinha") == 0) {
                scan_quote_string(search_criteria[j].value);
            } else {
                scanf("%s", search_criteria[j].value);
                if (strcmp(search_criteria[j].value, "NULO") == 0) search_criteria[j].value[0] = '\0';
            }
            if (strcmp(search_criteria[j].name, "codEstacao") == 0 && search_criteria[j].value[0] != '\0') {
                target_codEstacao = atoi(search_criteria[j].value);
            }
        }

        // --- 2. LÊ CRITÉRIOS DE ATUALIZAÇÃO (p) ---
        int p; scanf("%d", &p);
        Criterion *update_criteria = malloc(p * sizeof(Criterion));
        
        for (int j = 0; j < p; j++) {
            scanf("%s", update_criteria[j].name);
            if (strcmp(update_criteria[j].name, "nomeEstacao") == 0 || strcmp(update_criteria[j].name, "nomeLinha") == 0) {
                scan_quote_string(update_criteria[j].value);
            } else {
                scanf("%s", update_criteria[j].value);
                if (strcmp(update_criteria[j].value, "NULO") == 0) update_criteria[j].value[0] = '\0';
            }
        }

        Record *record = new_record();

        if (target_codEstacao != -1) {
            // ========= BUSCA INDEXADA =========
            int target_rrn = search_index(index_filename, target_codEstacao);
            if (target_rrn != -1) {
                fseek(bin, HEADER_SIZE + target_rrn * RECORD_SIZE, SEEK_SET);
                if (read_record_binary(bin, record)) {
                    if (!is_removed(record) && match_record(record, search_criteria, m)) {
                        int old_cod, new_cod;
                        apply_updates(record, update_criteria, p, &old_cod, &new_cod);
                        
                        // Volta o ponteiro e sobrescreve o registro na mesma posição
                        fseek(bin, HEADER_SIZE + target_rrn * RECORD_SIZE, SEEK_SET);
                        write_record_binary(bin, record);
                        
                        // Atualiza o índice caso a chave tenha mudado
                        if (old_cod != new_cod) {
                            remove_from_index(index_entries, &num_entries, old_cod);
                            add_to_index(&index_entries, &num_entries, new_cod, target_rrn);
                        }
                    }
                }
            }
        } else {
            // ========= BUSCA SEQUENCIAL =========
            fseek(bin, HEADER_SIZE, SEEK_SET);
            int current_rrn = 0;
            while (read_record_binary(bin, record)) {
                if (!is_removed(record) && match_record(record, search_criteria, m)) {
                    int old_cod, new_cod;
                    apply_updates(record, update_criteria, p, &old_cod, &new_cod);
                    
                    // Volta o ponteiro e sobrescreve o registro na mesma posição
                    fseek(bin, HEADER_SIZE + current_rrn * RECORD_SIZE, SEEK_SET);
                    write_record_binary(bin, record); 
                    
                    // Atualiza o índice caso a chave tenha mudado
                    if (old_cod != new_cod) {
                        remove_from_index(index_entries, &num_entries, old_cod);
                        add_to_index(&index_entries, &num_entries, new_cod, current_rrn);
                    }
                }
                current_rrn++;
            }
        }
        delete_record(&record);
        free(search_criteria);
        free(update_criteria);
    }

    // Atualiza os contadores de controle do cabeçalho frente às atualizações
    fix_header_counters(bin, header, drift_st, drift_pr);

    // Grava cabeçalho final
    write_header_binary(bin, header);
    update_header_status_binary(bin, '1');

    // Regrava índice atualizado
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