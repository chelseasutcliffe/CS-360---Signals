CC = gcc
CCFLAGS = -g -Wall -pedantic
EXEC = warn
OBJS = warn.o

${EXEC}: ${OBJS}
	${CC} ${CCFLAGS} -o ${EXEC} ${OBJS}

warn.o: warn.c
	${CC} ${CCFLAGS} -ansi -c warn.c

run: ${EXEC}
	./${EXEC}

clean:
	rm -f ${EXEC} ${OBJS}