#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM 100000

typedef enum Estado { LIVRE, OCUPADO, REMOVIDO } Estado;

typedef struct {
    char nome[51];
    char cpf[12];
    float nota;
    Estado estado;
} Registro;

int calcularHash(char* cpf) {
    char chaveStr[10];
    strncpy(chaveStr, cpf, 9);
    chaveStr[9] = '\0';
    int chave = atoi(chaveStr);

    srand(chave);
    return rand() % TAM;
}

int inserir(Registro* tabela, Registro novo) {
    int pos = calcularHash(novo.cpf);

    for (int i = 0; i < TAM; i++) {
        int j = (pos + i) % TAM;

        if (tabela[j].estado == OCUPADO && strcmp(tabela[j].cpf, novo.cpf) == 0) {
            printf("CPF já existe na tabela.\n");
            return 0;
        }

        if (tabela[j].estado == LIVRE || tabela[j].estado == REMOVIDO) {
            tabela[j] = novo;
            tabela[j].estado = OCUPADO;
            return 1;
        }
    }

    return 0;
}

int consultar(Registro* tabela, char* cpf, Registro* resultado) {
    int pos = calcularHash(cpf);

    for (int i = 0; i < TAM; i++) {
        int j = (pos + i) % TAM;
        if (tabela[j].estado == OCUPADO && strcmp(tabela[j].cpf, cpf) == 0) {
            *resultado = tabela[j];
            return 1;
        }
        if (tabela[j].estado == LIVRE) break;
    }

    return 0;
}

int remover(Registro* tabela, char* cpf) {
    int pos = calcularHash(cpf);

    for (int i = 0; i < TAM; i++) {
        int j = (pos + i) % TAM;
        if (tabela[j].estado == OCUPADO && strcmp(tabela[j].cpf, cpf) == 0) {
            tabela[j].estado = REMOVIDO;
            return 1;
        }
        if (tabela[j].estado == LIVRE) 
            break;
    }

    return 0;
}

Registro* carregarRegistros(int* total, char* arquivo) {
    FILE* f = fopen(arquivo, "rb");
    if (!f) {
        printf("Erro ao abrir registros.dat");
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    long tam = ftell(f);
    rewind(f);

    *total = tam / sizeof(Registro);
    Registro* lista = malloc(tam);
    fread(lista, sizeof(Registro), *total, f);
    fclose(f);
    return lista;
}

void salvarHash(Registro* tabela, char* arquivo) {
    FILE* f = fopen(arquivo, "wb");
    if (!f) {
        printf("Erro ao salvar hash.dat");
        return;
    }
    fwrite(tabela, sizeof(Registro), TAM, f);
    fclose(f);
}

void menu(Registro* tabela) {
    int opcao;
    char cpf[12];
    Registro r;

    do {
        printf("\n--- MENU HASH ---\n");
        printf("1. Consultar CPF\n");
        printf("2. Remover CPF\n");
        printf("3. Inserir novo registro\n");
        printf("4. Sair\n");
        printf("Escolha: ");
        scanf("%d", &opcao);
        getchar();

        switch (opcao) {
            case 1:
                printf("Digite o CPF para buscar: ");
                fgets(cpf, 12, stdin);
                cpf[strcspn(cpf, "\n")] = '\0';

                if (consultar(tabela, cpf, &r)) {
                    printf("\nRegistro encontrado:\n");
                    printf("Nome: %s\n", r.nome);
                    printf("CPF: %s\n", r.cpf);
                    printf("Nota: %.2f\n", r.nota);
                } else {
                    printf("Registro não encontrado.\n");
                }
                break;

            case 2:
                printf("Digite o CPF para remover: ");
                fgets(cpf, 12, stdin);
                cpf[strcspn(cpf, "\n")] = '\0';

                if (remover(tabela, cpf)){
                    printf("Removido com sucesso.\n");
                }
                else{
                    printf("CPF não encontrado.\n");
                }
                break;

            case 3:
                printf("Digite nome: ");
                fgets(r.nome, 51, stdin);
                r.nome[strcspn(r.nome, "\n")] = '\0';

                printf("Digite CPF (11 dígitos): ");
                fgets(r.cpf, 12, stdin);
                r.cpf[strcspn(r.cpf, "\n")] = '\0';

                printf("Digite nota: ");
                scanf("%f", &r.nota);
                getchar();

                if (inserir(tabela, r)){
                    printf("Inserido com sucesso.\n");
                }
                else{
                    printf("Tabela cheia. Inserção falhou.\n");
                }
                break;

            case 4:
                printf("Saindo...\n");
                break;

            default:
                printf("Opção inválida.\n");
        }
    } while (opcao != 4);
}

int main() {
    int total;
    Registro* tabelaHash = calloc(TAM, sizeof(Registro));
    Registro* registros = carregarRegistros(&total, "./registros/registros.dat");

    for (int i = 0; i < total; i++) {
        inserir(tabelaHash, registros[i]);
    }
    free(registros);

    menu(tabelaHash);

    salvarHash(tabelaHash, "hash.dat");
    free(tabelaHash);
    return 0;
}
