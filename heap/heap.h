#ifndef HEAP_H
#define HEAP_H

#include <stdio.h>

typedef struct {
    char nome[51];
    char cpf[12];
    float nota;
} Registro;

#define REG_TAM sizeof(Registro)

int inserir(FILE* arq, Registro *novo, int tam);
int removerTopo(FILE* arq, int tam);
void consultarTopo(FILE* arq, int tam);
void menuHeap(FILE* arq, int tam);
void imprimeHeap(FILE *arq, int tam);
int geraHeap();


#endif