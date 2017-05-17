#pragma once

#include "File.h"

#define FIFO_ENTRADA "/tmp/entrada"
#define FIFO_REJEITADOS "/tmp/rejeitados"

void init_time();
double get_time_mili();
void milisleep(double ms);

// ------------------

struct Request {
    unsigned id;
    enum gender_e { MALE='M', FEMALE='F' } gender;
    unsigned duration;
};

void log_request(F* f,struct Request* req,char* state,pid_t pid,pthread_t tid,int useTid);
void put_request(F* f,struct Request* req);
int get_request(F* f,struct Request* req);

// ------------------

struct Counter {
    char name[128];
    unsigned f,m;
};

struct Stats {
    struct Counter total;
    struct Counter accepted;
    struct Counter rejected;
    struct Counter discarded;
};

void initCounter(struct Counter* c,char* name);
void printRequestCounts(struct Counter* c);
void initStats(struct Stats* s);
void incMale(struct Counter* c);
void incFemale(struct Counter* c);
void computeTotal(struct Stats* s);
