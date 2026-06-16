
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
            break;
	}

	return 0;
}
