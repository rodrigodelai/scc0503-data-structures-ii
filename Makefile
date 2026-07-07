MAIN = main.c
UTIL = query_create.c query_select.c header.c record.c utils.c index.c query_update.c graph.c graph_adjacency.c graph_path.c graph_mst.c graph_cycles.c
BINARY = executable

all:
	gcc -Wall -g ${MAIN} ${UTIL} -o ${BINARY}

run:
	./${BINARY}

go: all run

debug:
	gcc -DDEBUG -Wall ${MAIN} ${UTIL} -o ${BINARY}

valgrind:
	valgrind --tool=memcheck --leak-check=full --track-origins=yes --show-leak-kinds=all --show-reachable=yes ./${BINARY}

clean:
	@rm *.o
