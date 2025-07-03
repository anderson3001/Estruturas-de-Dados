#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    char nome[51];
    char cpf[12];
    float nota;
} Registro;

void gerarNome(char *nome) {
    char letras[] = "abcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < 50; i++)
        nome[i] = letras[rand() % 26];
    nome[50] = '\0';
}

void gerarCPF(char *cpf) {
    for (int i = 0; i < 11; i++)
        cpf[i] = '0' + rand() % 10;
    cpf[11] = '\0';
}

void gerarRegistros(char *arquivo, int qtd) {
    FILE *f = fopen(arquivo, "wb");
    Registro r;
    for (int i = 0; i < qtd; i++) {
        gerarNome(r.nome);
        gerarCPF(r.cpf);
        r.nota = rand() % 101;
        fwrite(&r, sizeof(Registro), 1, f);
    }
    fclose(f);
}

int main() {
    srand(time(NULL));
    gerarRegistros("registros.dat", 10000);
    return 0;
}
