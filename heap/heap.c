#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REG_TAM sizeof(Registro)

typedef struct {
    char nome[51];
    char cpf[12];
    float nota;
} Registro;

int pai(int i) {
    return i / 2;
}

int esq(int i) {
    return i * 2;
}

int dir(int i) {
    return i * 2 + 1;
}

Registro* leRegistro(FILE *arq) {
    Registro *reg = (Registro*)malloc(sizeof(Registro));
    if (!reg) return NULL;

    if (fread(reg, sizeof(Registro), 1, arq) != 1) {
        free(reg);
        return NULL;
    }
    return reg;
}

void escreveReg(FILE *arq, Registro *reg) {
    if (arq == NULL || reg == NULL) 
    return;
    fwrite(reg, sizeof(Registro), 1, arq);
}

void subir(FILE* arq, int i) {
    if (i <= 1) return;

    int posPai = pai(i);
    if (posPai < 1) return;

    fseek(arq, (i - 1) * REG_TAM, SEEK_SET);
    Registro *reg = leRegistro(arq);
    if (!reg) return;

    fseek(arq, (posPai - 1) * REG_TAM, SEEK_SET);
    Registro *regPai = leRegistro(arq);
    if (!regPai) {
        free(reg);
        return;
    }

    if (reg->nota > regPai->nota) {
        fseek(arq, (i - 1) * REG_TAM, SEEK_SET);
        escreveReg(arq, regPai);

        fseek(arq, (posPai - 1) * REG_TAM, SEEK_SET);
        escreveReg(arq, reg);
        fflush(arq);

        free(reg);
        free(regPai);
        subir(arq, posPai);
    } else {
        free(reg);
        free(regPai);
    }
}

void descer(FILE* arq, int i, int tam) {
    Registro *reg = NULL, *regEsq = NULL, *regDir = NULL;
    int e = esq(i);
    int d = dir(i);
    
    fseek(arq, (i - 1) * REG_TAM, SEEK_SET);
    reg = leRegistro(arq);
    if (!reg) return;

    if (e <= tam) {
        fseek(arq, (e - 1) * REG_TAM, SEEK_SET);
        regEsq = leRegistro(arq);
    }

    if (d <= tam) {
        fseek(arq, (d - 1) * REG_TAM, SEEK_SET);
        regDir = leRegistro(arq);
    }

    int maior_pos = i;
    Registro* regMaior = reg;

    if (regEsq && regEsq->nota > regMaior->nota) {
        maior_pos = e;
        regMaior = regEsq;
    }
    if (regDir && regDir->nota > regMaior->nota) {
        maior_pos = d;
        regMaior = regDir;
    }

    if (maior_pos != i) {
        fseek(arq, (i - 1) * REG_TAM, SEEK_SET);
        escreveReg(arq, regMaior);

        fseek(arq, (maior_pos - 1) * REG_TAM, SEEK_SET);
        escreveReg(arq, reg);
        fflush(arq);

        descer(arq, maior_pos, tam);
    }

    free(reg);
    if (regEsq) free(regEsq);
    if (regDir) free(regDir);
}

int inserir(FILE* arq, Registro *novo, int tam) {
    tam++;
    fseek(arq, (tam - 1) * REG_TAM, SEEK_SET);
    escreveReg(arq, novo);
    fflush(arq);
    subir(arq, tam);

    return tam;
}

int removerTopo(FILE* arq, int tam) {
    if (tam < 1) {
        printf("Heap vazio.\n");
        return tam;
    }

    fseek(arq, (tam- 1) * REG_TAM, SEEK_SET);
    Registro *ultimo = leRegistro(arq);
    if (!ultimo) {
        return tam;
    }

    fseek(arq, 0, SEEK_SET);
    escreveReg(arq, ultimo);
    fflush(arq);

    tam--;

    free(ultimo);

    descer(arq, 1, tam);

    return tam;
}

void consultarTopo(FILE* arq, int tam) {
    if (tam < 1) {
        printf("Heap vazio.\n");
        return;
    }

    fseek(arq, 0, SEEK_SET);
    Registro *reg = leRegistro(arq);
    if (!reg) {
        printf("Erro ao ler o topo.\n");
        return;
    }

    printf("\n[TOPO DO HEAP]\n");
    printf("Nome: %s\n", reg->nome);
    printf("CPF: %s\n", reg->cpf);
    printf("Nota: %.2f\n", reg->nota);

    free(reg);
}

void menuHeap(FILE* arq, int tam) {
    int opcao;
    Registro novo;

    do {
        printf("\n--- MENU HEAP ---\n");
        printf("1. Consultar topo\n");
        printf("2. Inserir novo registro\n");
        printf("3. Remover topo\n");
        printf("4. Salvar e sair\n");
        printf("Escolha: ");
        scanf("%d", &opcao);
        getchar();

        switch (opcao) {
            case 1:
                consultarTopo(arq, tam);
                break;
            case 2:
                printf("Nome: ");
                fgets(novo.nome, 51, stdin);
                novo.nome[strcspn(novo.nome, "\n")] = '\0';

                printf("CPF: ");
                fgets(novo.cpf, 12, stdin);
                novo.cpf[strcspn(novo.cpf, "\n")] = '\0';

                printf("Nota: ");
                scanf("%f", &novo.nota);
                getchar();

                tam = inserir(arq, &novo, tam);
                printf("Registro inserido com sucesso.\n");
                break;
            case 3:
                tam = removerTopo(arq, tam);
                break;
            case 4:
                printf("Salvando e encerrando.\n");
                fflush(arq);
                return;
            default:
                printf("Opção inválida.\n");
        }

    } while (1);
}
void imprimeHeap(FILE *arq, int tam){

    if (arq == NULL) return;

    Registro *reg = NULL;

    printf("\n");
    fseek(arq, 0, SEEK_SET);
    
    for (int i = 1; i <= tam; i++){
        Registro *reg = leRegistro(arq);
        if (reg == NULL) break;
        printf("\nRegistro %d= { CPF= %s , NOME= %s, NOTA= %.2f}",i, reg->cpf, reg->nome, reg->nota);
        free(reg);
    }
}

int geraHeap(){
    int tam = 0;
    FILE *arq = fopen("./registros/registros.dat", "rb+");
    FILE *arqHeap = fopen("./heap/heap.dat", "wb+");

    if (arq == NULL) {
        printf("ERRO: Nao foi possivel abrir './registros/registros.dat'. Verifique se o arquivo e a pasta existem.\n");
        if (arqHeap) fclose(arqHeap);
        return -1;
    }
    if (arqHeap == NULL) {
        printf("ERRO: Nao foi possivel criar './heap/heap.dat'.\n");
        fclose(arq);
        return -1;
    }
    
    for (int i = 0; i < 10000; i++){
        Registro *reg = leRegistro(arq);

        if (reg == NULL) {
            break;
        }

        tam = inserir(arqHeap, reg, tam);
        free(reg);
    }
    fclose(arq);
    fclose(arqHeap);

    return tam;
}

int main() {
    FILE* arq = fopen("./heap/heap.dat", "r+b"); 

    if (arq == NULL) {
        printf("Arquivo heap.dat nao encontrado. Gerando uma nova heap...\n");

        int registros = geraHeap(); 
        if (registros < 0) {
            printf("Ocorreu um erro durante a geracao da heap. Encerrando.\n");
            return 1;
        }

        printf("Heap gerada com sucesso com %d registros.\n", registros);

        arq = fopen("./heap/heap.dat", "r+b");
        if (arq == NULL) {
            printf("Erro critico: Nao foi possivel abrir a heap mesmo apos a geracao.\n");
        }
           
    } else {
        printf("Heap existente encontrada. Carregando...\n");
    }
    fseek(arq, 0, SEEK_END);
    long tam_bytes = ftell(arq);
    int nHeap = (int)(tam_bytes / REG_TAM); 

    printf("Heap aberta com %d elementos.\n", nHeap);

    menuHeap(arq, nHeap);

    fclose(arq);
    return 0;
}
