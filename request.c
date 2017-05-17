#include "File.h"
#include "sauna.h"

#include <stdio.h>
#include <string.h>
#include <sys/times.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define MAXLEN_MSG 128

// TIME ---------------------------

static clock_t START_TICK;
static double TICKS_SEC;

static double ticks_to_mili(clock_t ticks) {
    return (((double)ticks)/TICKS_SEC)*1E3;
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

void milisleep(double ms) {
    if (ms == 0.0) { return; }
    struct timespec rqtp;
    rqtp.tv_sec = 0;
    rqtp.tv_nsec = (long)(ms*1E6);
    while (nanosleep(&rqtp,&rqtp) != 0) {
        if (errno == EINVAL) {
            printf("Sleep error\n");
        }
    }
}

// --------------------------------

void log_request(F* f,struct Request* req,char* state,pid_t pid,pthread_t tid,int useTid) {
    char msg[MAXLEN_MSG];
    if (!useTid) {
        sprintf(msg,"%.2f - %05d - %05d: %c - %05d - %s\n",
                get_time_mili(),
                pid,
                req->id,
                req->gender==MALE? 'M' : 'F',
                req->duration,
                state);
    } else {
        sprintf(msg,"%.2f - %05d - %05ld - %05d: %c - %05d - %s\n",
                get_time_mili(),
                pid,
                tid,
                req->id,
                req->gender==MALE? 'M' : 'F',
                req->duration,
                state);
    }
    F_printstring(f,msg);
}

void put_request(F* f,struct Request* req) {
    char msg[MAXLEN_MSG];
    sprintf(msg,"%d %c %d\n",
            req->id,
            req->gender==MALE? 'M' : 'F',
            req->duration);
    F_printstring(f,msg);
}

int get_request(F* f,struct Request* req) {
    char buf[128];
    F_readstring(f,buf);
    if (buf[0] == '\0') {
        return 1;
    }
    char c;
    if (sscanf(buf,"%u %c %u",&(req->id),&c,&(req->duration)) != 3) {
        printf("ERRO NO FIFO: sscanf\n");
        return 2;
    }
    if (c == 'M') {
        req->gender = MALE;
    } else if (c == 'F') {
        req->gender = FEMALE;
    } else {
        printf("ERRO NO FIFO: género\n");
    }
    return 0;
}

