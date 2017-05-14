#include "sauna.h"


void printRequestCounts(char* msg,struct Counter* c) {
    printf("Pedidos %s (total): %u\n",msg,c->m+c->f);
    printf("Pedidos %s (masculino): %u\n",msg,c->m);
    printf("Pedidos %s (feminino): %u\n",msg,c->f);
}

void incMale(struct Counter* c) { ++c->m; }

void incFemale(struct Counter* c) { ++c->f; }
