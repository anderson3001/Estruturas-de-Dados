#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gerar.h"

void gerarNome(char *nome) {
    char consoantes[] = "bcdfghjklmnpqrstvwxyz";
    char vogais[] = "aeiou";
    int tamanho = 6 + rand() % 5;

    for (int i = 0; i < tamanho; i++) {
        if (i % 2 == 0) {
            nome[i] = consoantes[rand() % (sizeof(consoantes) - 1)];
        } else {
            nome[i] = vogais[rand() % (sizeof(vogais) - 1)];
        }
    }
    nome[tamanho] = '\0';
}

void gerarCPF(int num, char *cpf) {
    for (int i = 0; i < 11; i++)
        cpf[i] = '0' + rand() % 10;
    cpf[11] = '\0';
}

void gerarRegistros(char *arquivo, int qtd) {
    FILE *f = fopen(arquivo, "wb");
    Registro r;
    for (int i = 0; i < qtd; i++) {
        gerarNome(r.nome);
        gerarCPF(i, r.cpf);
        r.nota = rand() % 101;
        fwrite(&r, sizeof(Registro), 1, f);
        printf("CPF gerado: %s    ", r.cpf);
        printf("Nome gerado: %s\n", r.nome);
    }
    fclose(f);
}

int main() {
    srand(time(NULL));
    gerarRegistros("registros/registros.dat", 10000);
    return 0;
}
