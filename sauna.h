#pragma once

#include "File.h"

#define FIFO_ENTRADA "/tmp/entrada"
#define FIFO_REJEITADOS "/tmp/rejeitados"

struct Request {
    unsigned id;
    unsigned duration;
    enum gender_e { MALE, FEMALE } gender;
};

struct Counter {
    unsigned f,m;
};

struct Stats {
    struct Counter total;
    struct Counter accepted;
    struct Counter rejected;
    struct Counter discarded;
};

enum time_e { SEC, MILI, MICRO };

static const int g_debug = 1;

// ------------------

void log_request(F* f,struct Request* req,char* state,pid_t pid,pthread_t tid,int useTid);

