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

#define LOG_FILE "/tmp/ger.%d"
#define MAXLEN_MSG 128

struct GeneratorOptions {
    unsigned numRequests;
    unsigned maxDuration;
    enum time_e time;
    F* logF;
    F* entF;

    struct GeneratorResult {
        unsigned maleReqsCount;
        unsigned femaleReqsCount;
    } result;
};


struct RejectedProcessorOptions {
    unsigned numAttempts;
    enum time_e time;
    F* logF;
    F* entF;

    struct RejectedProcessorResult {
        unsigned acceptedMaleReqsCount;
        unsigned acceptedFemaleReqsCount;
        unsigned rejectMaleReqsCount;
        unsigned rejectFemaleReqsCount;
        unsigned discardMaleReqsCount;
        unsigned discardFemaleReqsCount;
    } result;
};


static int g_finish = 0;
static pid_t g_pid;

struct Request genRandomRequest(unsigned maxDuration) {
    struct Request req;
    req.id = 0;
    req.duration = 10;
    req.gender = MALE;
    return req;
}

void* generator_thread(void* args) {
    struct GeneratorOptions* opt = (struct GeneratorOptions*)args;

    unsigned count = 0;
    for (int id=0; id < opt->numRequests; id++) {
        struct Request req = genRandomRequest(opt->maxDuration);
        req.id = id;
        if (req.gender == MALE) {
            ++(opt->result.maleReqsCount);
        } else {
            ++(opt->result.femaleReqsCount);
        }
        put_request(opt->entF,&req);
        log_request(opt->logF,&req,"PEDIDO",g_pid,0,0);
    }

    g_finish = 1;
    return NULL;
}

void* rejected_requests_processor_thread(void* args) {
    struct RejectedProcessorOptions* opt = 
        (struct RejectedProcessorOptions*)args;

    F* f = F_new_unbuffered(FIFO_REJEITADOS,RW_READ,CONC_FALSE);
    int fd = open(FIFO_REJEITADOS,O_RDONLY);
    if (fd < 0) {
        printf("Erro a abrir fifo\n");
        g_finish = 1;
        return NULL;
    }

    // TODO: 
    while (1) {
        //struct Request req = get_request();
        //put_request(opt->reqF,&req,"PEDIDO");
        if (g_finish) {
            break;
        }
    }

    close(fd);
    return NULL;
}

void printRequestCounts(char* msg,unsigned m,unsigned f) {
    printf("Pedidos %s (total)\n",msg,m+f);
    printf("Pedidos %s (masculino)\n",msg,m);
    printf("Pedidos %s (feminino)\n",msg,f);
}


int main(int argc,char* argv[])
{
    g_pid = getpid();

    /* Ficheiros */

    F* fent = F_new_unbuffered(FIFO_ENTRADA,RW_WRITE,CONC_TRUE);
    if (!fent) { return 1; }

    char logname[128];
    sprintf(logname,LOG_FILE,g_pid);
    F* flog =  F_new_buffered(logname,"w",CONC_TRUE);
    if (!flog) { return 2; } 

    /* opcoes */

    if(argc != 4) {
        printf("Uso: gerador <n. pedidos> <max. utilização> <un. tempo>\n");
        printf("  <n. pedidos>: número de pedidos gerado\n");
        printf("  <max. utilização>: tempo máximo de execução\n");
        printf("  <un. tempo>: segundo \'s\', mili \'m\', ou micro \'u\'\n");
    }
    enum time_e time_opt = SEC;

    struct GeneratorOptions opts1;
    opts1.numRequests = 100;
    opts1.maxDuration = 10;
    opts1.time = time_opt;
    opts1.logF = flog;
    opts1.entF = fent;

    struct RejectedProcessorOptions opts2;
    opts2.time = time_opt;
    opts2.numAttempts = 3;
    opts2.logF = flog;
    opts2.entF = fent;

    init_time();

    /* threads */

    pthread_t t1,t2;
    pthread_create(&t1,NULL,generator_thread,&opts1);
    pthread_create(&t2,NULL,rejected_requests_processor_thread,&opts2);

    /* resultados */

    pthread_join(t1,NULL);
    printRequestCounts("gerados",
            opts1.result.maleReqsCount,
            opts1.result.femaleReqsCount);

    pthread_join(t2,NULL);
    printRequestCounts("aceitados",
            opts2.result.acceptedMaleReqsCount,
            opts2.result.acceptedFemaleReqsCount);
    printRequestCounts("rejeitados",
            opts2.result.rejectMaleReqsCount,
            opts2.result.rejectFemaleReqsCount);
    printRequestCounts("descartados",
            opts2.result.discardMaleReqsCount,
            opts2.result.discardFemaleReqsCount);

    F_destroy(fent);
    F_destroy(flog);
    return 0;
}
