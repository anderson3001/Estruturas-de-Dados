#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void subir(Registro* heap, int i) {
    int j = pai(i);
    if (j >= 1 && heap[i].nota > heap[j].nota) {
        Registro temp = heap[i];
        heap[i] = heap[j];
        heap[j] = temp;
        subir(heap, j);
    }
}

void descer(Registro* heap, int i, int n) {
    int e = esq(i);
    int d = dir(i);
    int maior = i;

    if (e <= n && heap[e].nota > heap[maior].nota){
        maior = e;
    }
    if (d <= n && heap[d].nota > heap[maior].nota){
        maior = d;
    }
    if (maior != i) {
        Registro temp = heap[i];
        heap[i] = heap[maior];
        heap[maior] = temp;
        descer(heap, maior, n);
    }
}

Registro* inserir(Registro* heap, Registro novo, int* n) {
    Registro* temp = realloc(heap, sizeof(Registro) * (*n + 2));
    if (!temp) {
        printf("Erro na alocação de memória.\n");
        exit(1);
    }
    heap = temp;
    (*n)++;
    heap[*n] = novo;
    subir(heap, *n);
    return heap;
}

void salvarHeap(Registro* heap, int n, const char* nomeArquivo) {
    FILE* f = fopen(nomeArquivo, "wb");
    if (!f) {
        printf("Erro ao salvar heap");
        return;
    }
    fwrite(&n, sizeof(int), 1, f);
    fwrite(heap + 1, sizeof(Registro), n, f);
    fclose(f);
}

Registro* carregarRegistrosOriginais(int* total, const char* nomeArquivo) {
    FILE* f = fopen(nomeArquivo, "rb");
    if (!f) {
        printf("Erro ao abrir registros.dat");
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    long tamanho = ftell(f);
    rewind(f);

    *total = tamanho / sizeof(Registro);
    Registro* lista = malloc(tamanho);
    fread(lista, sizeof(Registro), *total, f);
    fclose(f);
    return lista;
}

Registro* removerTopo(Registro* heap, int* n) {
    if (*n == 0) {
        printf("Heap já está vazio.\n");
        return heap;
    }

    printf("Removendo registro: %s (nota %.2f)\n", heap[1].nome, heap[1].nota);
    heap[1] = heap[*n];
    (*n)--;
    heap = realloc(heap, sizeof(Registro) * (*n + 1));
    descer(heap, 1, *n);
    return heap;
}

void consultarTopo(Registro* heap, int n) {
    if (n == 0) {
        printf("Heap vazio.\n");
        return;
    }
    printf("\n[TOPO DO HEAP]\n");
    printf("Nome: %s\n", heap[1].nome);
    printf("CPF: %s\n", heap[1].cpf);
    printf("Nota: %.2f\n", heap[1].nota);
}

void menuHeap(Registro* heap, int* n) {
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
                consultarTopo(heap, *n);
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

                heap = inserir(heap, novo, n);
                printf("Registro inserido com sucesso.\n");
                break;
            case 3:
                heap = removerTopo(heap, n);
                break;
            case 4:
                salvarHeap(heap, *n, "heap.dat");
                printf("Heap salvo. Encerrando.\n");
                break;
            default:
                printf("Opção inválida.\n");
        }

    } while (opcao != 4);
}

int main() {
    int totalRegistros;
    Registro* registros = carregarRegistrosOriginais(&totalRegistros, "./registros/registros.dat");

    Registro* heap = malloc(sizeof(Registro));
    int nHeap = 0;

    for (int i = 0; i < totalRegistros; i++) {
        heap = inserir(heap, registros[i], &nHeap);
    }

    free(registros);

    menuHeap(heap, &nHeap);

    free(heap);
    return 0;
}
