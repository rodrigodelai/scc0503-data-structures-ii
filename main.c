
/*  Disciplina: SCC0503 – Algoritmos e Estruturas de Dados II
		Aluno: Miguel Lima
		Nº USP: 13830720
		Trabalho Prático 1
*/

#include "queries.h"
#include "index.h"
#include "graph.h"

int main() {
	int queryOption = read_integer();
	int rrn, criteria;
	char *csv_filename;
	char *bin_filename;

	switch (queryOption) {
		case 1:
			csv_filename = read_string();
			bin_filename = read_string();
			create_from_csv(csv_filename, bin_filename);
			break;
		case 2:
			bin_filename = read_string();
			select_all(bin_filename);
			break;
		case 3:
			bin_filename = read_string();
			criteria = read_integer();
			select_where(bin_filename, criteria);
			break;
		case 4:
			bin_filename = read_string();
			rrn = read_integer();
			select_by_rrn(bin_filename, rrn);
			break;
		case 5:
    		bin_filename = read_string();
   			char *index_filename = read_string();
    		create_index(bin_filename, index_filename);
   			delete_string(&index_filename); // liberar a memória da string
   			delete_string(&bin_filename);   // liberar a memória do nome do arquivo de dados
    		break;
		default:
			printf("Invalid query option\n");
			break;
		case 6:
            bin_filename = read_string();
            char *index_filename_6 = read_string(); // Nome diferente para evitar confusão com o case 5
            // O número de buscas pode não vir na linha de comando. Se estiver
            // ausente, usamos -1 para indicar "ler buscas até o fim da entrada".
            int num_searches;
            if (!read_int_same_line(&num_searches)) num_searches = -1;
            select_where_indexed(bin_filename, index_filename_6, num_searches);
            delete_string(&index_filename_6);
            delete_string(&bin_filename);
            break;
		case 7:
            bin_filename = read_string();
            char *index_filename_7 = read_string();
            int num_removals = read_integer();
            
            // Chama a função delete_records para realizar as remoções e atualizar o índice
            delete_records(bin_filename, index_filename_7, num_removals);
            
            // Libera a memória da string alocada para o nome do índice
            delete_string(&index_filename_7);
            delete_string(&bin_filename);
            break;
		case 8:
            bin_filename = read_string();
            char *index_filename_8 = read_string();
            int num_inserts = read_integer();
            
            insert_records(bin_filename, index_filename_8, num_inserts);

            delete_string(&index_filename_8);
            delete_string(&bin_filename);
            break;
		case 9:
            bin_filename = read_string();
            char *index_filename_9 = read_string();
            int num_updates = read_integer();
            
            update_records(bin_filename, index_filename_9, num_updates);

            delete_string(&index_filename_9);
            delete_string(&bin_filename);
            break;
		case 10: {
            // [10] Grafo em listas de adjacencias. Le o arquivo de dados e o
            // nome do arquivo de indice (consumido da entrada, nao utilizado).
            bin_filename = read_string();
            char *index_filename_10 = read_string();
            graph_adjacency_list(bin_filename);
            delete_string(&index_filename_10);
            delete_string(&bin_filename);
            break;
        }
		case 11: {
            // [11] Caminho mais curto entre origem e destino. Le: arquivo de
            // dados, arquivo de indice (ignorado), e os pares nomeCampo/valor
            // de origem e destino (valores entre aspas).
            bin_filename = read_string();
            char *index_filename_11 = read_string();
            char *field_origin = read_string();      // "nomeEstacao"
            char origin_value[128];
            scan_quote_string(origin_value);
            char *field_dest = read_string();        // "nomeEstacao"
            char dest_value[128];
            scan_quote_string(dest_value);
            graph_shortest_path(bin_filename, origin_value, dest_value);
            delete_string(&field_origin);
            delete_string(&field_dest);
            delete_string(&index_filename_11);
            delete_string(&bin_filename);
            break;
        }
		case 12: {
            // [12] Arvore geradora minima percorrida em profundidade a partir
            // da estacao de origem informada.
            bin_filename = read_string();
            char *index_filename_12 = read_string();
            char *field_origin = read_string();      // "nomeEstacao"
            char origin_value[128];
            scan_quote_string(origin_value);
            graph_mst_dfs(bin_filename, origin_value);
            delete_string(&field_origin);
            delete_string(&index_filename_12);
            delete_string(&bin_filename);
            break;
        }
		case 13: {
            // [13] Contagem de ciclos simples a partir da estacao de origem.
            bin_filename = read_string();
            char *index_filename_13 = read_string();
            char *field_origin = read_string();      // "nomeEstacao"
            char origin_value[128];
            scan_quote_string(origin_value);
            graph_count_cycles(bin_filename, origin_value);
            delete_string(&field_origin);
            delete_string(&index_filename_13);
            delete_string(&bin_filename);
            break;
        }
	}

	return 0;
}
