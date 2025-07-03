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
    heap = realloc(heap, sizeof(Registro) * (*n + 2));
    (*n)++;
    heap[*n] = novo;
    subir(heap, *n);
    return heap;
}

void salvarHeap(Registro* heap, int n, const char* nomeArquivo) {
    FILE* f = fopen(nomeArquivo, "wb");
    if (!f) {
        perror("Erro ao salvar heap");
        return;
    }
    fwrite(&n, sizeof(int), 1, f);
    fwrite(heap + 1, sizeof(Registro), n, f);
    fclose(f);
}

Registro* carregarRegistrosOriginais(int* total, const char* nomeArquivo) {
    FILE* f = fopen(nomeArquivo, "rb");
    if (!f) {
        perror("Erro ao abrir registros.dat");
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

int main() {
    int totalRegistros;
    Registro* registros = carregarRegistrosOriginais(&totalRegistros, "./registros/registros.dat");

    Registro* heap = malloc(sizeof(Registro));
    int nHeap = 0;

    for (int i = 0; i < totalRegistros; i++) {
        heap = inserir(heap, registros[i], &nHeap);
    }

    free(registros);

    consultarTopo(heap, nHeap);

    salvarHeap(heap, nHeap, "heap.dat");

    free(heap);
    return 0;
}
