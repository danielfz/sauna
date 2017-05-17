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
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>

#define LOG_FILE "/tmp/bal.%d"

#define DEBUG 1

sem_t semMale;
sem_t semFemale;
int g_finish = 0;

void* sauna_user(void* args) {
    struct Request* req = (struct Request*)args;
    milisleep((double)req->duration);
    if (DEBUG) {
        printf("- %d (M=%d) a utilizar sauna por %u ms\n",
                req->id,req->gender==MALE,req->duration);
    }
    sem_t* sem = req->gender==MALE ? &semMale : &semFemale; 
    sem_post(sem);
    free(req);
    return NULL;
}

void sauna_manager(
        F* fin,
        F* fout,
        F* flog,
        const unsigned N)
{
    struct Stats stats;
    initStats(&stats);

    sem_init(&semMale,0,N);
    sem_init(&semFemale,0,N);

    while (!g_finish) {
        struct Request* reqPtr = malloc(sizeof(struct Request));

        if (get_request(fin,reqPtr) != 0) {
            /* Esta não é a melhor maneira de fechar o programa.
             * Não sabiamos se todas as nossas leituras seriam 'blocking',
             * pelo que pusemos o read() num ciclo, mesmo enquanto retorna 0
             * que é o valor que devemos esperar quando o pipe fecha.
             * Em retrospectiva, poderíamos ter usado este valor 0 para
             * terminar.
             *
             * Neste método, tentamos escrever para o fifo /tmp/rejeitados.
             * Quando gerador fechar, este fifo fecha, e a sua escrita faz
             * com que o programa receba um sinal SIGPIPE, que é usado para
             * terminar (g_finish).
             */
            F_printstring(fout,"test pipe\n");
            continue;
        }
        log_request(flog,reqPtr,"RECEBIDO",getpid(),pthread_self(),1);

        int male = reqPtr->gender == MALE;
        int n;
        sem_t *sem = male ? &semMale : &semFemale;
        sem_getvalue(sem,&n);
        if (n > 0) {
            sem_wait(sem);
            pthread_t t;
            pthread_create(&t,NULL,sauna_user,reqPtr);
            if (male) {
                incMale(&(stats.accepted));
            } else {
                incFemale(&(stats.accepted));
            }
            log_request(flog,reqPtr,"RECEBIDO",getpid(),t,1);
        } else {
            if (male) {
                incMale(&(stats.rejected));
            } else {
                incFemale(&(stats.rejected));
            }
            printf("rejeitado: n=%d, M=%d\n",n,reqPtr->gender==MALE);
            put_request(fout,reqPtr);
            log_request(flog,reqPtr,"REJEITADO",getpid(),pthread_self(),1);
        }
    }

    int n;
    do { sem_getvalue(&semMale,&n);   } while(n < N);
    do { sem_getvalue(&semFemale,&n); } while(n < N);

    printRequestCounts(&(stats.accepted));
    printRequestCounts(&(stats.rejected));
    computeTotal(&stats);
    printRequestCounts(&(stats.total));
}

// -------------------------

void sigpipe_handler(int arg) { 
    g_finish = 1; 
}

int unlink_and_mkfifo(const char*name) {
    unlink(name);
    if (mkfifo(name,0660) != 0) { printf("Erro a criar fifo\n"); return -1; }
    return 0;
}

int main(int argc,char* argv[])
{
    unsigned numLugares = 100;
    if (argc == 1) {
        numLugares = 10;
    } else if (argc == 2) {
        numLugares = atoi(argv[1]);
    }
    printf("SAUNA ABERTA! Lugares=%d\n",numLugares);

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
    //printf("Sauna: fifo entrada aberto\n");

    F* fout = F_new_unbuffered(FIFO_REJEITADOS,RW_WRITE,CONC_FALSE);
    if (!fout) { return 1; }
    //printf("Sauna: fifo rejeitados aberto\n");

    char logname[128];
    sprintf(logname,LOG_FILE,getpid());
    F* flog =  F_new_buffered(logname,"w",CONC_TRUE);
    if (!flog) { return 2; } 

    struct sigaction action;
    action.sa_handler = sigpipe_handler;
    action.sa_flags = 0;
    sigemptyset(&(action.sa_mask));
    if (sigaction(SIGPIPE,&action,NULL) != 0) {
        printf("Erro ao instalar sigpipe hander\n");
        return 1;
    }

    init_time();
    sauna_manager(fin,fout,flog,numLugares);

    F_destroy(fin);
    F_destroy(fout);
    F_destroy(flog);
    return 0;
}

