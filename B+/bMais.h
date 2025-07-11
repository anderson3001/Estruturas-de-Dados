#ifndef BMAIS_H
#define BMAIS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INDICE_FILENAME "./B+/indices.idx"
#define DADOS_FILENAME "./B+/folhas.dat"
#define METADADOS_FILENAME "./B+/meta.dat"
#define REGISTROS_ORIGEM "./registros/registros.dat"

#define ORDEM 4 
#define MAX_CHAVES (2 * ORDEM)
#define MIN_CHAVES ORDEM
#define TRUE 1
#define FALSE 0

typedef enum { NAO_PROMOVE, PROMOVE } StatusPromocao;
typedef enum { OPERACAO_OK, UNDERFLOW } StatusOperacao;

typedef struct {
    char nome[51];
    char cpf[12];
    float nota;
} Registro;

typedef struct {
    long pos_raiz;
    int raiz_eh_folha;
} Metadados;

typedef struct {
    int aponta_para_folha;
    int num_chaves;
    int chaves[MAX_CHAVES];
    long ponteiros_nos[MAX_CHAVES + 1];
} NoInterno;

typedef struct {
    int num_chaves;
    int chaves[MAX_CHAVES];
    Registro dados[MAX_CHAVES];
    long proxima_folha;
} NoFolha;

void menu(FILE *fmeta, FILE *fidx, FILE *ffolha);
void cargaInicial(FILE *fmeta, FILE *fidx, FILE *ffolha);

void inserir(FILE *fmeta, FILE *fidx, FILE *ffolha, int chave, Registro r);
StatusPromocao inserir_recursivo(FILE *fidx, FILE *ffolha, long pos_atual, int eh_folha_atual, int chave, Registro r, int *chave_promovida, long *ponteiro_promovido);
void inserir_em_no_folha(NoFolha *no, int chave, Registro r);
void inserir_em_no_interno(NoInterno *no, int chave, long ponteiro);
void split_folha(FILE *ffolha, NoFolha *no_antigo, int chave_ins, Registro r_ins, int *chave_promovida, long *ponteiro_novo_no);
void split_interno(FILE *fidx, NoInterno *no_antigo, int chave_ins, long ponteiro_ins, int *chave_promovida, long *ponteiro_novo_no);

int buscar(FILE *fmeta, FILE *fidx, FILE *ffolha, int chave, Registro *r_encontrado);
void consultarPelaChave(FILE *fmeta, FILE *fidx, FILE *ffolha, const char *cpf_completo);

void remover(FILE *fmeta, FILE *fidx, FILE *ffolha, int chave);
StatusOperacao remover_recursivo(FILE *fidx, FILE *ffolha, long pos_atual, int eh_folha_atual, int chave);
void tratar_underflow(FILE *fidx, FILE *ffolha, NoInterno* pai, int pos_filho_underflow, long pos_pai);

void ler_metadados(FILE *fmeta, Metadados *meta);
void salvar_metadados(FILE *fmeta, Metadados meta);
int extrairChave(const char *cpf);


#endif