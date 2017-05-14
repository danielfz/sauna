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

void* sauna_user(void* args) {
}

void sauna_manager(F* fin,F* fout) {
    char buf[512];
    for (int i=0; i < 10; i++) {
        F_readstring(fin,buf);
        printf("readstring: %s\n",buf);
    }
}

// -------------------------


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

    F* fin = F_new_unbuffered(FIFO_ENTRADA,RW_READ,CONC_FALSE);
    if (!fin) { return 1; }
    printf("Sauna: fifo entrada aberto\n");

    F* fout = F_new_unbuffered(FIFO_REJEITADOS,RW_WRITE,CONC_FALSE);
    if (!fout) { return 1; }
    printf("Sauna: fifo rejeitados aberto\n");

    init_time();
    sauna_manager(fin,fout);

    F_destroy(fin);
    F_destroy(fout);
    return 0;
}

