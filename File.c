#include "File.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static F* F_new(const char* name,const int isConcurrent) {
    F* f = malloc(sizeof(F));
    if (!f) { printf("Erro de memória\n"); return NULL; }
    strncpy(f->name,name,F_FILENAME_SIZE);
    f->isBuffered = 0;
    f->isConcurrent = isConcurrent;
    f->fd = -1;
    f->fp = NULL;
    if (f->isConcurrent) { pthread_mutex_init(&(f->mutex),NULL); }
    return f;
}

F* F_new_buffered(const char* name,const char* mode,int isConc) {
    FILE* fp = fopen(name,mode);
    if (!fp) { printf("Erro a abrir ficheiro\n"); return NULL; }

    F* f = F_new(name,isConc);
    if (!f) { return NULL; }
    f->isBuffered = 1;
    f->fp = fp;
    return f;
}

F* F_new_unbuffered(const char* name,const int write,int isConc) {
    int flags = O_CREAT | (write ? O_WRONLY : O_RDONLY);
    int fd = open(name,flags,0666);
    if (fd < 0) { printf("Erro a abrir ficheiro\n"); return NULL; }

    F* f = F_new(name,isConc);
    if (!f) { return NULL; }
    f->isBuffered = 0;
    f->fd = fd;
    return f;
}

F* F_new_fifo(const char* name,const int write,int isConc) {
    unlink(name);
    if (mkfifo(name,0666) != 0) {
        printf("Erro a criar fifo\n");
        return NULL;
    }
    int fd = open(name,write ? O_WRONLY : O_RDONLY);;
    if (fd < 0) { 
        printf("Erro a abrir fifo\n");
        printf("Nome: %s\n",name);
        return NULL; 
    }

    F* f = F_new(name,isConc);
    if (!f) { return NULL; }
    f->isBuffered = 0;
    f->fd = fd;
    return f;
}

void F_destroy(F* f) {
    if (f->isBuffered) {
        fclose(f->fp);
    } else {
        close(f->fd);
    }
    free(f);
}

void F_printstring(F* f,char* msg) {
    if (f->isConcurrent) { pthread_mutex_lock(&f->mutex); }
    if (f->isBuffered) {
        fprintf(f->fp,msg);
    } else {
        write(f->fd,msg,strlen(msg));
        /*
        // teste
        int n = strlen(msg);
        for (int i=0; i < n; i++) {
            write(f->fd,msg+i,1);
        }
        */
    }
    if (f->isConcurrent) { pthread_mutex_unlock(&f->mutex); }
}

size_t F_readstring(F* f,char* buf) {
    if (f->isConcurrent) { pthread_mutex_lock(&f->mutex); }
    size_t count = 0;
    if (f->isBuffered) {
        // TODO
    } else {
        do {
            while (read(f->fd,buf+count,1) == 0)
                ;
        } while (buf[count++] != '\n');
        buf[count-1] = '\0';
    }
    if (f->isConcurrent) { pthread_mutex_unlock(&f->mutex); }
    return count;
}
