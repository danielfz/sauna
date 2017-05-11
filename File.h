#pragma once

#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>

#define F_FILENAME_SIZE 64

typedef struct {
    char name[F_FILENAME_SIZE];
    int isBuffered;
    int isConcurrent;
    int fd;
    FILE* fp;
    pthread_mutex_t mutex;
} F;

enum RW_e { RW_READ = 0, RW_WRITE };
enum CONC_e { CONC_FALSE = 0, CONC_TRUE };

F* F_new_unbuffered(const char* name,const int write,int isConc);

F* F_new_fifo(const char* name,const int write,int isConc);

F* F_new_buffered(const char* name,const char* mode,int isConc);

void F_destroy(F* cf);

void F_printstring(F* cf,char* msg);

size_t F_readstring(F* cf,char* buf);
