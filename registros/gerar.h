#ifndef GERAR_H
#define GERAR_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char nome[51];
    char cpf[12];
    float nota;
} Registro;

void gerarNome(char *nome);
void gerarCPF(int num, char *cpf);
void gerarRegistros(char *arquivo, int qtd);


#endif