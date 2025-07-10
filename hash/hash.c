#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"


int calcularHash(char* cpf) {
    char chaveStr[10];
    strncpy(chaveStr, cpf, 9);
    chaveStr[9] = '\0';
    int chave = atoi(chaveStr);

    srand(chave);
    return rand() % TAM;
}

void inicializar_hash(FILE* f) {
    if (!f) {
        perror("Erro: ponteiro de arquivo nulo passado para inicializar_hash");
        return;
    }

    rewind(f);

    Registro reg_livre = {0};
    reg_livre.estado = LIVRE;

    printf("Criando e formatando o arquivo de hash com %d posicoes...\n", TAM);
    for (int i = 0; i < TAM; i++) {
        fwrite(&reg_livre, sizeof(Registro), 1, f);
    }

    fflush(f);

    printf("Arquivo de hash formatado com sucesso.\n");
}



int inserir(FILE* f, Registro novo) {
    if (!f) return -1;

    int pos_inicial = calcularHash(novo.cpf);

    for (int i = 0; i < TAM; i++) {
        int j = (pos_inicial + i) % TAM;
        long offset = j * sizeof(Registro);
        fseek(f, offset, SEEK_SET);
        
        Registro reg_atual;
        fread(&reg_atual, sizeof(Registro), 1, f);

        if (reg_atual.estado == OCUPADO && strcmp(reg_atual.cpf, novo.cpf) == 0) {
            return 0;
        }

        if (reg_atual.estado == LIVRE || reg_atual.estado == REMOVIDO) {
            novo.estado = OCUPADO;
            fseek(f, offset, SEEK_SET);
            fwrite(&novo, sizeof(Registro), 1, f);
            fflush(f);
            return 1;
        }
    }
    return -1;
}


int consultar(FILE* f, char* cpf, Registro* resultado) {
    if (!f) return 0;

    int pos_inicial = calcularHash(cpf);

    for (int i = 0; i < TAM; i++) {
        int j = (pos_inicial + i) % TAM;
        long offset = j * sizeof(Registro);
        fseek(f, offset, SEEK_SET);
        
        Registro reg_atual;
        fread(&reg_atual, sizeof(Registro), 1, f);

        if (reg_atual.estado == LIVRE) {
            return 0;
        }

        if (reg_atual.estado == OCUPADO && strcmp(reg_atual.cpf, cpf) == 0) {
            *resultado = reg_atual;
            return 1;
        }
    }

    return 0;
}

int remover(FILE* f, char* cpf) {
    if (!f) return 0;

    int pos_inicial = calcularHash(cpf);

    for (int i = 0; i < TAM; i++) {
        int j = (pos_inicial + i) % TAM;
        long offset = j * sizeof(Registro);
        fseek(f, offset, SEEK_SET);

        Registro reg_atual;
        fread(&reg_atual, sizeof(Registro), 1, f);

        if (reg_atual.estado == LIVRE) {
            return 0;
        }

        if (reg_atual.estado == OCUPADO && strcmp(reg_atual.cpf, cpf) == 0) {
            reg_atual.estado = REMOVIDO;
            fseek(f, offset, SEEK_SET);
            fwrite(&reg_atual, sizeof(Registro), 1, f);
            fflush(f);
            return 1;
        }
    }

    return 0;
}

void menu(FILE* f) {
    // Verificação de segurança: se o arquivo não estiver aberto, encerra a função.
    if (!f) {
        printf("Erro critico: O arquivo de hash nao esta aberto para o menu.\n");
        return;
    }

    int opcao;
    char cpf[12];
    Registro r;

    do {
        printf("\n--- MENU HASH ---\n");
        printf("1. Consultar por CPF\n");
        printf("2. Remover por CPF\n");
        printf("3. Inserir novo registro\n");
        printf("4. Sair\n");
        printf("--------------------------------------\n");
        printf("Escolha uma opcao: ");

        if (scanf("%d", &opcao) != 1) {
            opcao = -1;
        }

        while(getchar() != '\n');

        switch (opcao) {
            case 1:
                printf("Digite o CPF para buscar (11 digitos): ");
                fgets(cpf, 12, stdin);
                cpf[strcspn(cpf, "\n")] = '\0';

                if (consultar(f, cpf, &r)) {
                    printf("\n>>> Registro Encontrado:\n");
                    printf("  Nome: %s\n", r.nome);
                    printf("  CPF:  %s\n", r.cpf);
                    printf("  Nota: %.2f\n", r.nota);
                } else {
                    printf(">>> Registro com CPF '%s' nao encontrado.\n", cpf);
                }
                break;

            case 2:
                printf("Digite o CPF para remover (11 digitos): ");
                fgets(cpf, 12, stdin);
                cpf[strcspn(cpf, "\n")] = '\0';
                
                if (remover(f, cpf)) {
                    printf(">>> Registro com CPF '%s' removido com sucesso.\n", cpf);
                } else {
                    printf(">>> CPF nao encontrado para remocao.\n");
                }
                break;

            case 3:
                printf("Inserindo novo registro:\n");
                printf("  Digite o nome: ");
                fgets(r.nome, 51, stdin);
                r.nome[strcspn(r.nome, "\n")] = '\0';

                printf("  Digite o CPF (11 digitos): ");
                fgets(r.cpf, 12, stdin);
                r.cpf[strcspn(r.cpf, "\n")] = '\0';

                printf("  Digite a nota: ");
                scanf("%f", &r.nota);
                while(getchar() != '\n');

                int resultado_insercao = inserir(f, r);
                if (resultado_insercao == 1) {
                    printf("Registro inserido com sucesso!\n");
                } else if (resultado_insercao == 0) {
                    printf("Falha: CPF '%s' ja existe na tabela.\n", r.cpf);
                } else {
                    printf("Falha: Tabela de hash esta cheia.\n");
                }
                break;

            case 4:
                printf("Saindo do menu...\n");
                break;

            default:
                printf("Opcao invalida. Por favor, tente novamente.\n");
                break;
        }
    } while (opcao != 4);
}

int main() {
    FILE *f_hash;

    f_hash = fopen(HASH_FILENAME, "rb+");

    if (f_hash == NULL) {
        printf("Arquivo '%s' nao encontrado.\n", HASH_FILENAME);

        f_hash = fopen(HASH_FILENAME, "wb+");
        if (f_hash == NULL) {
            perror("Nao foi possivel criar o arquivo de hash");
            exit(1);
        }
      
        inicializar_hash(f_hash);

        FILE* f_registros = fopen("./registros/registros.dat", "rb");
        if (!f_registros) {
            printf("AVISO: Nao foi possivel abrir './registros/registros.dat' para a carga inicial.\n");
            printf("A tabela de hash foi criada vazia.\n");
        } else {
            printf("Populando a tabela de hash com dados de './registros/registros.dat'...\n");
            
            RegistroArquivo reg_origem; 
            int contador = 0;
            while(fread(&reg_origem, sizeof(RegistroArquivo), 1, f_registros) == 1) {
                Registro reg_destino;
                strcpy(reg_destino.nome, reg_origem.nome);
                strcpy(reg_destino.cpf, reg_origem.cpf);
                reg_destino.nota = reg_origem.nota;
                reg_destino.estado = OCUPADO;

                inserir(f_hash, reg_destino);

                if (++contador % 500 == 0) {
                    printf("... %d registros inseridos.\n", contador);
                }
            }
            fclose(f_registros);
            printf("Carga inicial concluida com %d registros.\n", contador);
        }
    }
    else {
        printf("Arquivo de hash '%s' encontrado. Carregando...\n", HASH_FILENAME);
    }

    menu(f_hash);

    printf("Encerrando e fechando o arquivo de hash.\n");
    fclose(f_hash); 
    
    return 0;
}
