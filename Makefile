MAIN = main.c
UTIL = queries.c header.c record.c utils.c
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
