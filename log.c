#include "File.h"
#include "sauna.h"

#include <stdio.h>
#include <string.h>

#define MAXLEN_MSG 128

void log_request(F* f,struct Request* req,char* state,pid_t pid,pthread_t tid,int useTid) {
    char msg[MAXLEN_MSG];
    int n = sprintf(msg,"log: <inst> - %05d - %05d: %c - %05d\n",
            pid,
            req->id,
            req->gender==MALE? 'M' : 'F',
            req->duration);
    if (useTid) {
        sprintf(msg+n-2," - %d\n",tid);
    }
    F_printstring(f,msg);

    if (g_debug) {
        printf(msg);
    }
}

