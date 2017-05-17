/* SOPE/MIEIC/FEUP - 2017
 * Gerador de pedidos para a sauna
 *
 * Log file: /tmp/ger.<pid>
 *
 * <instant> - <pid> - <id>: <F/M> - <duration> - <PEDIDO/REJEITADO/DESCARTADO>
 */
#include "sauna.h"
#include "File.h"

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#define LOG_FILE "/tmp/ger.%d"
#define MAXLEN_MSG 128

struct Options {
    unsigned numRequests;
    unsigned maxDuration;
    unsigned numAttempts;
    F* logF;
    F* entF;
    F* rejF;
    struct Stats stats;
    double waitTime;
};

static int g_finish = 0;
static double timeOfLastRequest;
static pid_t g_pid;

struct Request genRandomRequest(unsigned maxDuration) {
    struct Request req;
    req.duration = rand() % maxDuration;
    req.gender = rand() % 2 ? MALE : FEMALE;
    return req;
}

void* generator_thread(void* args) {
    struct Options* opt = (struct Options*)args;
    initStats(&(opt->stats));

    for (unsigned id=0; id < opt->numRequests; id++) {
        printf("- Pedido %d gerado\n",id);
        struct Request req = genRandomRequest(opt->maxDuration);
        double nextRequestTimeMs = rand() % (opt->maxDuration/2);
        milisleep(nextRequestTimeMs);
        req.id = id;
        if (req.gender == MALE) {
            incMale(&(opt->stats.total));
        } else {
            incFemale(&(opt->stats.total));
        }
        put_request(opt->entF,&req);
        log_request(opt->logF,&req,"PEDIDO",g_pid,0,0);
    }
    timeOfLastRequest = get_time_mili();
    g_finish = 1;
    printf("Gerador concluído\n");
    return NULL;
}

void* rejected_requests_processor_thread(void* args) {
    struct Options* opt = (struct Options*)args;
    initStats(&(opt->stats));

    unsigned char* rejList = malloc(opt->numRequests*sizeof(unsigned char));
    if (!rejList) {
        printf("Erro: malloc\n");
        exit(1);
    }
    for (int i=0; i < opt->numRequests; i++) { rejList[i] = 0; }

    /* Esperar até ter a certeza que todos os clientes foram processados */
    timeOfLastRequest = get_time_mili();
    while (1) {
        if (g_finish && ((get_time_mili() - timeOfLastRequest) >= opt->waitTime)) {
            break;
        }
        struct Request req;
        if (get_request(opt->rejF,&req) == 0) {
            if (++rejList[req.id] == 3) {
                printf("descartado\n");
                if (req.gender==MALE) {
                    incMale(&(opt->stats.rejected));
                } else {
                    incFemale(&(opt->stats.rejected));
                }
                log_request(opt->logF,&req,"DESCARTADO",g_pid,0,0);
            } else {
                printf("reenviado\n");
                double t = get_time_mili();
                if (g_finish && t > timeOfLastRequest) {
                    timeOfLastRequest = t;
                }
                if (req.gender==MALE) {
                    incMale(&(opt->stats.discarded));
                } else {
                    incFemale(&(opt->stats.discarded));
                }
                put_request(opt->entF,&req);
            }
        }
    }

    printf("Processador concluído\n");
    return NULL;
}

int main(int argc,char* argv[])
{
    if(argc != 3) {
        printf("Uso: gerador <n. pedidos> <max. utilização>\n");
        printf("  <n. pedidos>: número de pedidos gerado\n");
        printf("  <max. utilização>: tempo máximo de execução\n");
        exit(1);
    }
    const unsigned numRequests = atoi(argv[1]);
    const unsigned maxDuration = atoi(argv[2]);

    srand(time(NULL));
    g_pid = getpid();

    /* Ficheiros */

    F* fent = F_new_unbuffered(FIFO_ENTRADA,RW_WRITE,CONC_TRUE);
    if (!fent) { return 1; }

    char logname[128];
    sprintf(logname,LOG_FILE,g_pid);
    F* flog =  F_new_buffered(logname,"w",CONC_TRUE);
    if (!flog) { return 2; } 

    /* opcoes */

    struct Options opts1;
    opts1.numRequests = numRequests;
    opts1.maxDuration = maxDuration;
    opts1.numAttempts = 3;
    opts1.logF = flog;
    opts1.entF = fent;

    struct Options opts2 = opts1;
    F* frej = F_new_unbuffered(FIFO_REJEITADOS,RW_READ,CONC_FALSE);
    if (!frej) { printf("erro\n"); return 3; }
    opts2.rejF = frej;
    opts2.waitTime = 3*maxDuration;

    init_time();

    /* threads */

    pthread_t t1,t2;
    pthread_create(&t1,NULL,generator_thread,&opts1);
    pthread_create(&t2,NULL,rejected_requests_processor_thread,&opts2);

    /* resultados */

    pthread_join(t1,NULL);
    printRequestCounts(&(opts1.stats.total));

    while ((get_time_mili() - timeOfLastRequest) < opts2.waitTime)
        ;
    pthread_cancel(t2);
    printRequestCounts(&(opts2.stats.rejected));
    printRequestCounts(&(opts2.stats.discarded));

    F_destroy(fent);
    F_destroy(flog);
    F_destroy(frej);
    return 0;
}
