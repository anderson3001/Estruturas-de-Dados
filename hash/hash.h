#ifndef HASH_H
#define HASH_H

#include <stdio.h>

#define TAM 100000
#define HASH_FILENAME "hash/hash.dat"

typedef enum Estado { LIVRE, OCUPADO, REMOVIDO } Estado;

typedef struct {
    char nome[51];
    char cpf[12];
    float nota;
    Estado estado;
} Registro;

typedef struct {
    char nome[51];
    char cpf[12];
    float nota;
} RegistroArquivo;


int calcularHash(char* cpf);
void inicializar_hash(FILE* f);
int inserir(FILE* f, Registro novo);
int consultar(FILE* f, char* cpf, Registro* resultado);
int remover(FILE* f, char* cpf);
void menu(FILE* f);


#endif