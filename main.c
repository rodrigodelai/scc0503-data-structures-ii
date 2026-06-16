
/*  Disciplina: SCC0503 – Algoritmos e Estruturas de Dados II
		Alunos: Rodrigo Borges Delai e Miguel Lima
		Nº USP: 11849074 e 13830720 (respectivamente)
		Trabalho 1: Sistema de Gerenciamento de Arquivo de Dados
*/

#include "queries.h"
#include "index.h"

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
            char *index_filename_6 = read_string(); // Nome diferente para evitar conflito de escopo dependendo da compilação
            int num_searches = read_integer();
            select_where_indexed(bin_filename, index_filename_6, num_searches);
            delete_string(&index_filename_6);
            delete_string(&bin_filename);
            break;
		case 7:
            bin_filename = read_string();
            char *index_filename_7 = read_string();
            int num_removals = read_integer();
            
            // Chama a função de deleção
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
	}

	return 0;
}
