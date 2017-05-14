#pragma once

#include "File.h"

#define FIFO_ENTRADA "/tmp/entrada"
#define FIFO_REJEITADOS "/tmp/rejeitados"

struct Request {
    unsigned id;
    enum gender_e { MALE, FEMALE } gender;
    unsigned duration;
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

void init_time();
double get_time_mili();

void log_request(F* f,struct Request* req,char* state,pid_t pid,pthread_t tid,int useTid);
void put_request(F* f,struct Request* req);
struct Request get_request(F* f);
