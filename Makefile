all: gerador sauna

File.o gerador.o sauna.o: File.h

OBJ_gerador=gerador.o File.o log.o

OBJ_sauna=sauna.o File.o log.o

gerador: ${OBJ_gerador}
	gcc -Wall -pthread ${OBJ_gerador} -o gerador

sauna: ${OBJ_sauna}
	gcc -Wall -pthread ${OBJ_sauna} -o sauna

.PHONY: all
