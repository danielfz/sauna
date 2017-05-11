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

#define LOG_FILE "/tmp/bal.%d"

struct Request get_request() {
}

void* sauna_user(void* args) {
}

void sauna_manager(F* fin,F* fout) {
    printf("sleeping...\n");
    sleep(10);
}

// -------------------------


/*
int open_fifo(const char* name,const int write) {
    int fd = open(name,write ? O_WRONLY : O_RDONLY);;
    if (fd < 0) { printf("Erro a abrir fifo\n"); }
    return fd;
}
*/

int unlink_and_mkfifo(const char*name) {
    unlink(name);
    if (mkfifo(name,0660) != 0) { printf("Erro a criar fifo\n"); return -1; }
    return 0;
}

int main(int argc,char* argv[])
{
    if (unlink_and_mkfifo(FIFO_REJEITADOS) || unlink_and_mkfifo(FIFO_ENTRADA)) {
        return 1;
    }

    /*
     * Ordem em que abrimos os fifos é importante.
     * Se abrirmos F1 e F2 no processo 1 e F2 e F1 no processo 2, o P1 fica
     * à espera que alguém abra F1 e o P2 à espera de F2.
     */

    F* f1 = F_new_unbuffered(FIFO_ENTRADA,RW_READ,CONC_FALSE);
    if (!f1) { return 1; }
    printf("Sauna: fifo entrada aberto\n");

    F* f2 = F_new_unbuffered(FIFO_REJEITADOS,RW_WRITE,CONC_FALSE);
    if (!f2) { return 1; }
    printf("Sauna: fifo rejeitados aberto\n");

    sauna_manager(f1,f2);

    F_destroy(f1);
    F_destroy(f2);
    return 0;
}

