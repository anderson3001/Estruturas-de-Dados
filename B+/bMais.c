#include "bMais.h"

void ler_metadados(FILE *fmeta, Metadados *meta) {
    fseek(fmeta, 0, SEEK_SET);
    if (fread(meta, sizeof(Metadados), 1, fmeta) == 0) {
        meta->pos_raiz = -1;
        meta->raiz_eh_folha = TRUE;
    }
}

void salvar_metadados(FILE *fmeta, Metadados meta) {
    fseek(fmeta, 0, SEEK_SET);
    fwrite(&meta, sizeof(Metadados), 1, fmeta);
}



void reescrever_no_folha(FILE *ffolha, NoFolha *n, long p) {
    fseek(ffolha, p, SEEK_SET);
    fwrite(n, sizeof(NoFolha), 1, ffolha);
}

void reescrever_no_interno(FILE *fidx, NoInterno *n, long p) {
    fseek(fidx, p, SEEK_SET);
    fwrite(n, sizeof(NoInterno), 1, fidx);
}

long escrever_novo_folha(FILE *ffolha, NoFolha *n) {
    fseek(ffolha, 0, SEEK_END);
    long p = ftell(ffolha);
    fwrite(n, sizeof(NoFolha), 1, ffolha);
    return p;
}

long escrever_novo_interno(FILE *fidx, NoInterno *n) {
    fseek(fidx, 0, SEEK_END);
    long p = ftell(fidx);
    fwrite(n, sizeof(NoInterno), 1, fidx);
    return p;
}

int extrairChave(const char *cpf) {
    char c[10] = {0};
    strncpy(c, cpf, 9);
    return atoi(c);
}

int buscar(FILE *fmeta, FILE *fidx, FILE *ffolha, int chave, Registro *r_encontrado) {
    Metadados meta;
    ler_metadados(fmeta, &meta);

    if (meta.pos_raiz == -1) return FALSE;

    long pos_atual = meta.pos_raiz;
    NoInterno no_interno;

    if (!meta.raiz_eh_folha) {
        while (1) {
            fseek(fidx, pos_atual, SEEK_SET);
            fread(&no_interno, sizeof(NoInterno), 1, fidx);
            if (no_interno.aponta_para_folha) break;

            int i = 0;
            while (i < no_interno.num_chaves && chave >= no_interno.chaves[i]) i++;
            pos_atual = no_interno.ponteiros_nos[i];
        }
        int i = 0;
        while (i < no_interno.num_chaves && chave >= no_interno.chaves[i]) i++;
        pos_atual = no_interno.ponteiros_nos[i];
    }
    
    NoFolha no_folha;
    fseek(ffolha, pos_atual, SEEK_SET);
    fread(&no_folha, sizeof(NoFolha), 1, ffolha);

    for (int i = 0; i < no_folha.num_chaves; i++) {
        if (no_folha.chaves[i] == chave) {
            *r_encontrado = no_folha.dados[i];
            return TRUE;
        }
    }
    
    return FALSE;
}

StatusPromocao inserir_recursivo(FILE *fidx, FILE *ffolha, long pos_atual, int eh_folha_atual, int chave, Registro r, int *chave_promovida, long *ponteiro_promovido) {
    if (eh_folha_atual) {
        NoFolha folha;
        fseek(ffolha, pos_atual, SEEK_SET);
        fread(&folha, sizeof(NoFolha), 1, ffolha);
        
        if (folha.num_chaves < MAX_CHAVES) {
            inserir_em_no_folha(&folha, chave, r);
            reescrever_no_folha(ffolha, &folha, pos_atual);
            return NAO_PROMOVE;
        } else {
            split_folha(ffolha, &folha, chave, r, chave_promovida, ponteiro_promovido);
            reescrever_no_folha(ffolha, &folha, pos_atual);
            return PROMOVE;
        }
    } else { // Nó interno
        NoInterno no_interno;
        fseek(fidx, pos_atual, SEEK_SET);
        fread(&no_interno, sizeof(NoInterno), 1, fidx);

        int i = 0;
        while (i < no_interno.num_chaves && chave >= no_interno.chaves[i]) i++;
        
        if (inserir_recursivo(fidx, ffolha, no_interno.ponteiros_nos[i], no_interno.aponta_para_folha, chave, r, chave_promovida, ponteiro_promovido) == PROMOVE) {
            if (no_interno.num_chaves < MAX_CHAVES) {
                inserir_em_no_interno(&no_interno, *chave_promovida, *ponteiro_promovido);
                reescrever_no_interno(fidx, &no_interno, pos_atual);
                return NAO_PROMOVE;
            } else {
                split_interno(fidx, &no_interno, *chave_promovida, *ponteiro_promovido, chave_promovida, ponteiro_promovido);
                reescrever_no_interno(fidx, &no_interno, pos_atual);
                return PROMOVE;
            }
        }
    }
    return NAO_PROMOVE;
}

void inserir(FILE *fmeta, FILE *fidx, FILE *ffolha, int chave, Registro r) {
    Metadados meta;
    ler_metadados(fmeta, &meta);

    if (meta.pos_raiz == -1) {
        NoFolha nova_folha;
        nova_folha.num_chaves = 1;
        nova_folha.chaves[0] = chave;
        nova_folha.dados[0] = r; // Insere o registro diretamente
        nova_folha.proxima_folha = -1;
        
        meta.pos_raiz = escrever_novo_folha(ffolha, &nova_folha);
        meta.raiz_eh_folha = TRUE;
        salvar_metadados(fmeta, meta);
        return;
    }

    int chave_promovida;
    long ponteiro_promovido;
    StatusPromocao status = inserir_recursivo(fidx, ffolha, meta.pos_raiz, meta.raiz_eh_folha, chave, r, &chave_promovida, &ponteiro_promovido);

    if (status == PROMOVE) {
        NoInterno nova_raiz;
        nova_raiz.num_chaves = 1;
        nova_raiz.chaves[0] = chave_promovida;
        nova_raiz.ponteiros_nos[0] = meta.pos_raiz;
        nova_raiz.ponteiros_nos[1] = ponteiro_promovido;
        nova_raiz.aponta_para_folha = meta.raiz_eh_folha;

        long nova_pos_raiz = escrever_novo_interno(fidx, &nova_raiz);
        
        meta.pos_raiz = nova_pos_raiz;
        meta.raiz_eh_folha = FALSE;
        salvar_metadados(fmeta, meta);
    }
}

void inserir_em_no_folha(NoFolha *no, int chave, Registro r) {
    int i = no->num_chaves;
    while (i > 0 && chave < no->chaves[i - 1]) {
        no->chaves[i] = no->chaves[i - 1];
        no->dados[i] = no->dados[i - 1];
        i--;
    }
    no->chaves[i] = chave;
    no->dados[i] = r;
    no->num_chaves++;
}

void inserir_em_no_interno(NoInterno *no, int chave, long ponteiro) {
    int i = no->num_chaves;
    while (i > 0 && chave < no->chaves[i - 1]) {
        no->chaves[i] = no->chaves[i - 1];
        no->ponteiros_nos[i + 1] = no->ponteiros_nos[i];
        i--;
    }
    no->chaves[i] = chave;
    no->ponteiros_nos[i + 1] = ponteiro;
    no->num_chaves++;
}

void split_folha(FILE *ffolha, NoFolha *no_antigo, int chave_ins, Registro r_ins, int *chave_promovida, long *ponteiro_novo_no) {
    int i;
    int temp_chaves[MAX_CHAVES + 1];
    Registro temp_dados[MAX_CHAVES + 1]; // Array temporário de Registros

    for (i = 0; i < MAX_CHAVES; i++) {
        temp_chaves[i] = no_antigo->chaves[i];
        temp_dados[i] = no_antigo->dados[i];
    }
    i = MAX_CHAVES;
    while (i > 0 && chave_ins < temp_chaves[i - 1]) {
        temp_chaves[i] = temp_chaves[i - 1];
        temp_dados[i] = temp_dados[i - 1];
        i--;
    }
    temp_chaves[i] = chave_ins;
    temp_dados[i] = r_ins;

    NoFolha novo_no = {0};
    novo_no.proxima_folha = no_antigo->proxima_folha;
    int meio = (MAX_CHAVES + 1) / 2;
    no_antigo->num_chaves = meio;
    novo_no.num_chaves = (MAX_CHAVES + 1) - meio;

    for (i = 0; i < no_antigo->num_chaves; i++) {
        no_antigo->chaves[i] = temp_chaves[i];
        no_antigo->dados[i] = temp_dados[i];
    }
    for (i = 0; i < novo_no.num_chaves; i++) {
        novo_no.chaves[i] = temp_chaves[meio + i];
        novo_no.dados[i] = temp_dados[meio + i];
    }

    *ponteiro_novo_no = escrever_novo_folha(ffolha, &novo_no);
    no_antigo->proxima_folha = *ponteiro_novo_no;
    *chave_promovida = novo_no.chaves[0];
}

void split_interno(FILE *fidx, NoInterno *no_antigo, int chave_ins, long ponteiro_ins, int *chave_promovida, long *ponteiro_novo_no) {
    int i;
    int temp_chaves[MAX_CHAVES + 1];
    long temp_ponteiros[MAX_CHAVES + 2];

    for (i = 0; i < MAX_CHAVES; i++) {
        temp_chaves[i] = no_antigo->chaves[i];
    }
    for (i = 0; i < MAX_CHAVES + 1; i++) {
        temp_ponteiros[i] = no_antigo->ponteiros_nos[i];
    }

    int pos = 0;
    while (pos < MAX_CHAVES && chave_ins > temp_chaves[pos]) pos++;
    
    for (i = MAX_CHAVES; i > pos; i--) temp_chaves[i] = temp_chaves[i-1];
    for (i = MAX_CHAVES + 1; i > pos + 1; i--) temp_ponteiros[i] = temp_ponteiros[i-1];
    
    temp_chaves[pos] = chave_ins;
    temp_ponteiros[pos + 1] = ponteiro_ins;

    NoInterno novo_no = {0};
    novo_no.aponta_para_folha = no_antigo->aponta_para_folha;
    int meio = ORDEM;
    *chave_promovida = temp_chaves[meio];
    
    no_antigo->num_chaves = meio;
    novo_no.num_chaves = MAX_CHAVES - meio;
    
    for(i=0; i < no_antigo->num_chaves; i++) {
        no_antigo->chaves[i] = temp_chaves[i];
        no_antigo->ponteiros_nos[i] = temp_ponteiros[i];
    }
    no_antigo->ponteiros_nos[no_antigo->num_chaves] = temp_ponteiros[no_antigo->num_chaves];

    for(i=0; i < novo_no.num_chaves; i++){
        novo_no.chaves[i] = temp_chaves[meio + 1 + i];
        novo_no.ponteiros_nos[i] = temp_ponteiros[meio + 1 + i];
    }
    novo_no.ponteiros_nos[novo_no.num_chaves] = temp_ponteiros[MAX_CHAVES + 1];

    *ponteiro_novo_no = escrever_novo_interno(fidx, &novo_no);
}

void consultarPelaChave(FILE *fmeta, FILE *fidx, FILE *ffolha, const char *cpf_completo) {
    int chave_busca = extrairChave(cpf_completo);
    Registro r;

    if (buscar(fmeta, fidx, ffolha, chave_busca, &r)) {
        printf("\n>>> Registro Encontrado:\n");
        printf("  Nome: %s\n", r.nome);
        printf("  CPF:  %s\n", r.cpf);
        printf("  Nota: %.2f\n", r.nota);
    } else {
        printf(">>> Nenhum registro encontrado para a chave %d.\n", chave_busca);
    }
}

void remover(FILE *fmeta, FILE *fidx, FILE *ffolha, int chave) {
    Metadados meta;
    ler_metadados(fmeta, &meta);
    if (meta.pos_raiz == -1) {
        printf("Árvore vazia. Nada a remover.\n");
        return;
    }

    remover_recursivo(fidx, ffolha, meta.pos_raiz, meta.raiz_eh_folha, chave);

    ler_metadados(fmeta, &meta);
    if (!meta.raiz_eh_folha) {
        NoInterno raiz;
        fseek(fidx, meta.pos_raiz, SEEK_SET);
        fread(&raiz, sizeof(NoInterno), 1, fidx);
        if (raiz.num_chaves == 0) {
            meta.pos_raiz = raiz.ponteiros_nos[0];
 
            if(raiz.aponta_para_folha){
                 meta.raiz_eh_folha = TRUE;
            } else {
                 meta.raiz_eh_folha = FALSE;
            }
            salvar_metadados(fmeta, meta);
        }
    }
}

StatusOperacao remover_recursivo(FILE *fidx, FILE *ffolha, long pos_atual, int eh_folha_atual, int chave) {
    if (eh_folha_atual) {
        NoFolha folha;
        fseek(ffolha, pos_atual, SEEK_SET);
        fread(&folha, sizeof(NoFolha), 1, ffolha);

        int pos_chave = -1;
        for (int i = 0; i < folha.num_chaves; i++) {
            if (folha.chaves[i] == chave) {
                pos_chave = i;
                break;
            }
        }

        if (pos_chave == -1) {
            printf("Chave %d não encontrada para remoção.\n", chave);
            return OPERACAO_OK;
        }

        for (int i = pos_chave; i < folha.num_chaves - 1; i++) {
            folha.chaves[i] = folha.chaves[i + 1];
            folha.dados[i] = folha.dados[i + 1];
        }
        folha.num_chaves--;
        reescrever_no_folha(ffolha, &folha, pos_atual);

        Metadados meta;
        FILE* fmeta_temp = fopen(METADADOS_FILENAME, "r+b");
        ler_metadados(fmeta_temp, &meta);
        fclose(fmeta_temp);

        if (pos_atual != meta.pos_raiz && folha.num_chaves < MIN_CHAVES) {
            return UNDERFLOW;
        }
        return OPERACAO_OK;

    } else {
        NoInterno no;
        fseek(fidx, pos_atual, SEEK_SET);
        fread(&no, sizeof(NoInterno), 1, fidx);

        int pos_descida = 0;
        while (pos_descida < no.num_chaves && chave >= no.chaves[pos_descida]) {
            pos_descida++;
        }

        if (remover_recursivo(fidx, ffolha, no.ponteiros_nos[pos_descida], no.aponta_para_folha, chave) == UNDERFLOW) {
            tratar_underflow(fidx, ffolha, &no, pos_descida, pos_atual);
        }

        Metadados meta;
        FILE* fmeta_temp = fopen(METADADOS_FILENAME, "r+b");
        ler_metadados(fmeta_temp, &meta);
        fclose(fmeta_temp);
        
        if (pos_atual != meta.pos_raiz && no.num_chaves < MIN_CHAVES) {
            return UNDERFLOW;
        }
        return OPERACAO_OK;
    }
}

void tratar_underflow(FILE *fidx, FILE *ffolha, NoInterno* pai, int pos_filho_underflow, long pos_pai) {
    long pos_filho = pai->ponteiros_nos[pos_filho_underflow];

    long pos_irmao_esq = (pos_filho_underflow > 0) ? pai->ponteiros_nos[pos_filho_underflow - 1] : -1;
    long pos_irmao_dir = (pos_filho_underflow < pai->num_chaves) ? pai->ponteiros_nos[pos_filho_underflow + 1] : -1;

    // 1. TENTAR REDISTRIBUIÇÃO
    
    // Tenta emprestar do irmão da DIREITA
    if (pos_irmao_dir != -1) {
        if (pai->aponta_para_folha) {
            NoFolha filho, irmao;
            fseek(ffolha, pos_filho, SEEK_SET); fread(&filho, sizeof(NoFolha), 1, ffolha);
            fseek(ffolha, pos_irmao_dir, SEEK_SET); fread(&irmao, sizeof(NoFolha), 1, ffolha);

            if (irmao.num_chaves > MIN_CHAVES) {
                filho.chaves[filho.num_chaves] = irmao.chaves[0];
                filho.dados[filho.num_chaves] = irmao.dados[0];
                filho.num_chaves++;
                pai->chaves[pos_filho_underflow] = irmao.chaves[1];
                irmao.num_chaves--;
                for (int i = 0; i < irmao.num_chaves; i++) {
                    irmao.chaves[i] = irmao.chaves[i+1];
                    irmao.dados[i] = irmao.dados[i+1];
                }
                reescrever_no_folha(ffolha, &filho, pos_filho);
                reescrever_no_folha(ffolha, &irmao, pos_irmao_dir);
                reescrever_no_interno(fidx, pai, pos_pai);
                return;
            }
        } else { // LÓGICA PARA NÓ INTERNO
            NoInterno filho, irmao;
            fseek(fidx, pos_filho, SEEK_SET); fread(&filho, sizeof(NoInterno), 1, fidx);
            fseek(fidx, pos_irmao_dir, SEEK_SET); fread(&irmao, sizeof(NoInterno), 1, fidx);

            if (irmao.num_chaves > MIN_CHAVES) {
                filho.chaves[filho.num_chaves] = pai->chaves[pos_filho_underflow];
                filho.ponteiros_nos[filho.num_chaves + 1] = irmao.ponteiros_nos[0];
                filho.num_chaves++;
                pai->chaves[pos_filho_underflow] = irmao.chaves[0];
                irmao.num_chaves--;
                for (int i = 0; i < irmao.num_chaves; i++) {
                    irmao.chaves[i] = irmao.chaves[i+1];
                    irmao.ponteiros_nos[i] = irmao.ponteiros_nos[i+1];
                }
                irmao.ponteiros_nos[irmao.num_chaves] = irmao.ponteiros_nos[irmao.num_chaves + 1];
                
                reescrever_no_interno(fidx, &filho, pos_filho);
                reescrever_no_interno(fidx, &irmao, pos_irmao_dir);
                reescrever_no_interno(fidx, pai, pos_pai);
                return;
            }
        }
    }

    // Tenta emprestar do irmão da ESQUERDA
    if (pos_irmao_esq != -1) {
        if (pai->aponta_para_folha) {
            NoFolha filho, irmao;
            fseek(ffolha, pos_filho, SEEK_SET); fread(&filho, sizeof(NoFolha), 1, ffolha);
            fseek(ffolha, pos_irmao_esq, SEEK_SET); fread(&irmao, sizeof(NoFolha), 1, ffolha);
            
            if (irmao.num_chaves > MIN_CHAVES) {
                for(int i = filho.num_chaves; i > 0; i--) {
                    filho.chaves[i] = filho.chaves[i-1];
                    filho.dados[i] = filho.dados[i-1];
                }
                filho.chaves[0] = irmao.chaves[irmao.num_chaves - 1];
                filho.dados[0] = irmao.dados[irmao.num_chaves - 1];
                filho.num_chaves++;
                irmao.num_chaves--;
                pai->chaves[pos_filho_underflow - 1] = filho.chaves[0];

                reescrever_no_folha(ffolha, &filho, pos_filho);
                reescrever_no_folha(ffolha, &irmao, pos_irmao_esq);
                reescrever_no_interno(fidx, pai, pos_pai);
                return;
            }
        } else { // LÓGICA PARA NÓ INTERNO
            NoInterno filho, irmao;
            fseek(fidx, pos_filho, SEEK_SET); fread(&filho, sizeof(NoInterno), 1, fidx);
            fseek(fidx, pos_irmao_esq, SEEK_SET); fread(&irmao, sizeof(NoInterno), 1, fidx);

            if (irmao.num_chaves > MIN_CHAVES) {
                for (int i = filho.num_chaves; i > 0; i--) filho.chaves[i] = filho.chaves[i-1];
                for (int i = filho.num_chaves + 1; i > 0; i--) filho.ponteiros_nos[i] = filho.ponteiros_nos[i-1];
                
                filho.chaves[0] = pai->chaves[pos_filho_underflow - 1];
                filho.ponteiros_nos[0] = irmao.ponteiros_nos[irmao.num_chaves];
                filho.num_chaves++;
                pai->chaves[pos_filho_underflow - 1] = irmao.chaves[irmao.num_chaves - 1];
                irmao.num_chaves--;

                reescrever_no_interno(fidx, &filho, pos_filho);
                reescrever_no_interno(fidx, &irmao, pos_irmao_esq);
                reescrever_no_interno(fidx, pai, pos_pai);
                return;
            }
        }
    }

    //SE NÃO FOI POSSÍVEL REDISTRIBUIR, FAZER A CONCATENAÇÃO

    // Tenta fundir com o irmão da ESQUERDA
    if (pos_irmao_esq != -1) {
        if(pai->aponta_para_folha){
            NoFolha filho, irmao;
            fseek(ffolha, pos_filho, SEEK_SET); fread(&filho, sizeof(NoFolha), 1, ffolha);
            fseek(ffolha, pos_irmao_esq, SEEK_SET); fread(&irmao, sizeof(NoFolha), 1, ffolha);

            for(int i=0; i<filho.num_chaves; i++){
                irmao.chaves[irmao.num_chaves + i] = filho.chaves[i];
                irmao.dados[irmao.num_chaves + i] = filho.dados[i];
            }
            irmao.num_chaves += filho.num_chaves;
            irmao.proxima_folha = filho.proxima_folha;

            reescrever_no_folha(ffolha, &irmao, pos_irmao_esq);
        } else { // LÓGICA PARA NÓ INTERNO
            NoInterno filho, irmao;
            fseek(fidx, pos_filho, SEEK_SET); fread(&filho, sizeof(NoInterno), 1, fidx);
            fseek(fidx, pos_irmao_esq, SEEK_SET); fread(&irmao, sizeof(NoInterno), 1, fidx);
            
            irmao.chaves[irmao.num_chaves] = pai->chaves[pos_filho_underflow - 1];
            irmao.num_chaves++;

            for(int i=0; i<filho.num_chaves; i++){
                irmao.chaves[irmao.num_chaves + i] = filho.chaves[i];
                irmao.ponteiros_nos[irmao.num_chaves + i] = filho.ponteiros_nos[i];
            }
            irmao.ponteiros_nos[irmao.num_chaves + filho.num_chaves] = filho.ponteiros_nos[filho.num_chaves];
            irmao.num_chaves += filho.num_chaves;

            reescrever_no_interno(fidx, &irmao, pos_irmao_esq);
        }
 
        for(int i = pos_filho_underflow - 1; i < pai->num_chaves - 1; i++){
            pai->chaves[i] = pai->chaves[i+1];
        }
        for (int i = pos_filho_underflow; i < pai->num_chaves; i++) {
            pai->ponteiros_nos[i] = pai->ponteiros_nos[i+1];
        }
        pai->num_chaves--;
        reescrever_no_interno(fidx, pai, pos_pai);

    } else { // Tenta fundir com o irmão da DIREITA
        if(pai->aponta_para_folha){
            NoFolha filho, irmao;
            fseek(ffolha, pos_filho, SEEK_SET); fread(&filho, sizeof(NoFolha), 1, ffolha);
            fseek(ffolha, pos_irmao_dir, SEEK_SET); fread(&irmao, sizeof(NoFolha), 1, ffolha);
            
            for(int i=0; i<irmao.num_chaves; i++){
                filho.chaves[filho.num_chaves + i] = irmao.chaves[i];
                filho.dados[filho.num_chaves + i] = irmao.dados[i];
            }
            filho.num_chaves += irmao.num_chaves;
            filho.proxima_folha = irmao.proxima_folha;

            reescrever_no_folha(ffolha, &filho, pos_filho);
        } else { // LÓGICA PARA NÓ INTERNO
            NoInterno filho, irmao;
            fseek(fidx, pos_filho, SEEK_SET); fread(&filho, sizeof(NoInterno), 1, fidx);
            fseek(fidx, pos_irmao_dir, SEEK_SET); fread(&irmao, sizeof(NoInterno), 1, fidx);

            filho.chaves[filho.num_chaves] = pai->chaves[pos_filho_underflow];
            filho.num_chaves++;

            for(int i=0; i<irmao.num_chaves; i++){
                filho.chaves[filho.num_chaves + i] = irmao.chaves[i];
                filho.ponteiros_nos[filho.num_chaves + i] = irmao.ponteiros_nos[i];
            }
            filho.ponteiros_nos[filho.num_chaves + irmao.num_chaves] = irmao.ponteiros_nos[irmao.num_chaves];
            filho.num_chaves += irmao.num_chaves;
            
            reescrever_no_interno(fidx, &filho, pos_filho);
        }

        for(int i = pos_filho_underflow; i < pai->num_chaves - 1; i++){
            pai->chaves[i] = pai->chaves[i+1];
        }
        for (int i = pos_filho_underflow + 1; i < pai->num_chaves; i++) {
            pai->ponteiros_nos[i] = pai->ponteiros_nos[i+1];
        }
        pai->num_chaves--;
        reescrever_no_interno(fidx, pai, pos_pai);
    }
}

void cargaInicial(FILE *fmeta, FILE *fidx, FILE *ffolha) {
    FILE* f_origem = fopen(REGISTROS_ORIGEM, "rb");
    if (!f_origem) {
        printf("AVISO: Nao foi possivel abrir '%s' para a carga inicial.\n", REGISTROS_ORIGEM);
        return;
    }
    
    printf("Populando a Árvore B+ com dados de '%s'...\n", REGISTROS_ORIGEM);
    Registro reg_origem;
    int contador = 0;
    while(fread(&reg_origem, sizeof(Registro), 1, f_origem) == 1) {
        int chave = extrairChave(reg_origem.cpf);
        inserir(fmeta, fidx, ffolha, chave, reg_origem); 
        if (++contador % 500 == 0) printf("... %d registros inseridos.\n", contador);
    }
    fclose(f_origem);
    printf("Carga inicial concluida com %d registros.\n", contador);
}

void menu(FILE *fmeta, FILE *fidx, FILE *ffolha) {
    if (!fidx) {
        printf("Erro critico: O arquivo de índice não está aberto para o menu.\n");
        return;
    }
    int opcao;
    char cpf_str[13];
    int chave;
    Registro r;
    do {
        printf("\n--- MENU ÁRVORE B+ ---\n");
        printf("1. Consultar por CPF\n");
        printf("2. Inserir novo registro\n");
        printf("3. Remover por CPF\n");
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
                fgets(cpf_str, 12, stdin);
                cpf_str[strcspn(cpf_str, "\n")] = '\0';
                consultarPelaChave(fmeta, fidx, ffolha, cpf_str);
                break;
            case 2:
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
                
                int chave = extrairChave(r.cpf);
                inserir(fmeta, fidx, ffolha, chave, r);
                printf("\n>>> Registro inserido com sucesso!\n");
                break;
            case 3:
                printf("Digite o CPF para remover (11 digitos): ");
                fgets(cpf_str, 13, stdin);
                cpf_str[strcspn(cpf_str, "\n")] = '\0';
                int chave_rem = extrairChave(cpf_str);
                remover(fmeta, fidx, ffolha, chave_rem);
                printf("\n>>> Operação de remoção para a chave %d concluída.\n", chave_rem);
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
    FILE *fidx, *ffolha, *fmeta;
    
    system("mkdir -p B+"); 
    
    fmeta = fopen(METADADOS_FILENAME, "r+b");
    fidx = fopen(INDICE_FILENAME, "r+b");
    ffolha = fopen(DADOS_FILENAME, "r+b");

    if (fmeta == NULL || fidx == NULL || ffolha == NULL) {
        printf("Um ou mais arquivos não encontrados. Criando novo conjunto...\n");
        if(fmeta) fclose(fmeta);
        if(fidx) fclose(fidx);
        if(ffolha) fclose(ffolha);

        fmeta = fopen(METADADOS_FILENAME, "wb+");
        fidx = fopen(INDICE_FILENAME, "wb+");
        ffolha = fopen(DADOS_FILENAME, "wb+");
        
        if (!fmeta || !fidx || !ffolha) {
            perror("Erro fatal ao criar arquivos");
            exit(1);
        }
        
        Metadados meta_inicial = {-1, TRUE};
        salvar_metadados(fmeta, meta_inicial);
        cargaInicial(fmeta, fidx, ffolha);
    } else {
        printf("Arquivos carregados com sucesso.\n");
    }

    menu(fmeta, fidx, ffolha);

    fclose(fidx);
    fclose(ffolha);
    fclose(fmeta);

    printf("Programa encerrado.\n");
    return 0;
}