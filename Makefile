CFLAGS=-Wall -Wshadow -Wpointer-arith

all: gerador sauna

File.o gerador.o sauna.o request.o: File.h sauna.h

OBJ_gerador=gerador.o File.o request.o

OBJ_sauna=sauna.o File.o request.o

gerador: ${OBJ_gerador}
	gcc ${CFLAGS} -pthread ${OBJ_gerador} -o gerador

sauna: ${OBJ_sauna}
	gcc ${CFLAGS} -pthread ${OBJ_sauna} -o sauna

.PHONY: all
