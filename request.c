#include "File.h"
#include "sauna.h"

#include <stdio.h>
#include <string.h>
#include <sys/times.h>
#include <unistd.h>

#define MAXLEN_MSG 128

// TIME ---------------------------

static clock_t START_TICK;
static double TICKS_SEC;

static double ticks_to_mili(clock_t ticks) {
    return (((double)ticks)/TICKS_SEC)*1E6;
}

void init_time() {
    struct tms buf;
    START_TICK = times(&buf);
    TICKS_SEC = (double)sysconf(_SC_CLK_TCK);
}

double get_time_mili() {
    struct tms buf;
    clock_t dt = times(&buf)-START_TICK;
    return ticks_to_mili(dt);
}

// --------------------------------

void log_request(F* f,struct Request* req,char* state,pid_t pid,pthread_t tid,int useTid) {
    char msg[MAXLEN_MSG];
    int n = sprintf(msg,"%.2f - %05d - %05d: %c - %05d\n",
            get_time_mili(),    // TODO
            pid,
            req->id,
            req->gender==MALE? 'M' : 'F',
            req->duration);
    if (useTid) {
        sprintf(msg+n-2," - %lu\n",tid);
    }
    F_printstring(f,msg);

    if (g_debug) {
        printf(msg);
    }
}

void put_request(F* f,struct Request* req) {
    char msg[MAXLEN_MSG];
    sprintf(msg,"%d %c %d\n",
            req->id,
            req->gender==MALE? 'M' : 'F',
            req->duration);
    F_printstring(f,msg);
}

struct Request get_request(F* f) {
    char buf[128];
    F_readstring(f,buf);
    struct Request req;
    char c;
    sscanf(buf,"%u %c %u",&(req.id),&c,&(req.duration));
    if (c == 'M') {
        req.gender = MALE;
    } else if (c == 'F') {
        req.gender = FEMALE;
    } else {
        printf("ERROR IN FIFO: gender\n");
    }
    return req;
}

