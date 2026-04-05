
/*  Disciplina: SCC0503 – Algoritmos e Estruturas de Dados II
		Alunos: Rodrigo Borges Delai e Miguel Lima
		Nº USP: 11849074 e 13830720 (respectivamente)
		Trabalho 1: Sistema de Gerenciamento de Arquivo de Dados
*/

#include "queries.h"

int main() {
	int queryOption = read_integer();

	switch (queryOption) {
		case 1:
			create_from_csv("./provided-files/estacoes.csv", "estacoes.bin");
			break;
		case 2:
			printf("Query 2 selected (SELECT ALL)\n");
			break;
		case 3:
			printf("Query 3 selected (SELECT WHERE)\n");
			break;
		case 4:
			printf("Query 4 selected (SELECT BY RRN)\n");
			break;
		default:
			printf("Invalid query option\n");
			break;
	}

	return 0;
}
