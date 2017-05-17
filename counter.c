#include "sauna.h"
#include <string.h>

void initCounter(struct Counter* c,char* name) {
    strcpy(c->name,name);
    c->f = 0;
    c->m = 0;
}

void printRequestCounts(struct Counter* c) {
    printf("Pedidos %s (total): %u\n",c->name,c->m+c->f);
    printf("Pedidos %s (masculino): %u\n",c->name,c->m);
    printf("Pedidos %s (feminino): %u\n",c->name,c->f);
}

void initStats(struct Stats* s) {
    initCounter(&(s->total),"total");
    initCounter(&(s->accepted),"recebidos");
    initCounter(&(s->rejected),"rejeitados");
    initCounter(&(s->discarded),"descartados");
}

void incMale(struct Counter* c)   { ++c->m; }
void incFemale(struct Counter* c) { ++c->f; }

void computeTotal(struct Stats* s) {
    s->total.m = (s->accepted.m + s->rejected.m + s->discarded.m);
    s->total.f = (s->accepted.f + s->rejected.f + s->discarded.f);
}
