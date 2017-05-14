/* SOPE/MIEIC/FEUP - 2017
 * Gestor da sauna.
 *
 *
 * Log file: /tmp/<pid>.pid
 */

#include "sauna.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdlib.h>

#define LOG_FILE "/tmp/bal.%d"

sem_t semMale;
sem_t semFemale;

void* sauna_user(void* args) {
    struct Request* req = (struct Request*)args;
    //sleep();
    sem_t* sem = req->gender==MALE ? &semMale : &semFemale; 
    sem_post(sem);
    free(req);
    return NULL;
}

void sauna_manager(
        F* fin,
        F* fout,
        F* flog,
        const unsigned numLugares)
{
    sem_init(&semMale,0,numLugares);
    sem_init(&semFemale,0,numLugares);

    for (int i=0; i < 10; i++) {
        struct Request* reqPtr = malloc(sizeof(struct Request));
        *reqPtr = get_request(fin);
        log_request(flog,reqPtr,"RECEBIDO",getpid(),0,1);

        int n;
        sem_t *sem = (reqPtr->gender == MALE) ? &semMale : &semFemale;
        sem_getvalue(sem,&n);
        if (n > 0) {
            sem_wait(sem);
            pthread_t t;
            pthread_create(&t,NULL,sauna_user,reqPtr);
            log_request(flog,reqPtr,"RECEBIDO",getpid(),t,1);
        } else {
            put_request(fout,reqPtr);
            log_request(flog,reqPtr,"REJEITADO",getpid(),0,1);
        }
    }

    int n;
    do { sem_getvalue(&semMale,&n);   } while(n < numLugares);
    do { sem_getvalue(&semFemale,&n); } while(n < numLugares);
}

// -------------------------


int unlink_and_mkfifo(const char*name) {
    unlink(name);
    if (mkfifo(name,0660) != 0) { printf("Erro a criar fifo\n"); return -1; }
    return 0;
}

int main(int argc,char* argv[])
{
    // TODO: ler num lugares

    const unsigned numLugares = 10;

    if (unlink_and_mkfifo(FIFO_REJEITADOS) || unlink_and_mkfifo(FIFO_ENTRADA)) {
        return 1;
    }

    /*
     * Ordem em que abrimos os fifos é importante.
     * Se abrirmos F1 e F2 no processo 1 e F2 e F1 no processo 2, o P1 fica
     * à espera que alguém abra F1 e o P2 à espera de F2.
     */

    F* fin = F_new_unbuffered(FIFO_ENTRADA,RW_READ,CONC_FALSE);
    if (!fin) { return 1; }
    printf("Sauna: fifo entrada aberto\n");

    F* fout = F_new_unbuffered(FIFO_REJEITADOS,RW_WRITE,CONC_FALSE);
    if (!fout) { return 1; }
    printf("Sauna: fifo rejeitados aberto\n");

    char logname[128];
    sprintf(logname,LOG_FILE,getpid());
    F* flog =  F_new_buffered(logname,"w",CONC_TRUE);
    if (!flog) { return 2; } 

    init_time();
    sauna_manager(fin,fout,flog,numLugares);

    F_destroy(fin);
    F_destroy(fout);
    F_destroy(flog);
    return 0;
}

