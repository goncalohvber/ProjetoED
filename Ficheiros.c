//
//  Ficheiros.c
//  ProjetoFinalED
//
//  Created by Gonçalo Henrique Viegas Bernardino on 11/05/2026.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "supermercado.h"

/* ================================================================
   Leitura de Ficheiros
   ================================================================ */

/* ================================================================
    Esta função lê a configuração. Ela primeiro abre o ficheiro e blablabla verifica se ele existe... Bom, ela faz scan das palavras no ficheiro dentro do ciclo while, quando chave = Max_Espera, ou o que andamos a procurar, ela lê o que está na frente do maximo de espera (que vai ser o máximo de espera HAHAAHAH) e é isso. ;)
   ================================================================ */
int lerConfiguracao(const char *ficheiro, Configuracao *config) {
    FILE *f = fopen(ficheiro, "r");
    if (!f) {
        printf("Erro: nao foi possivel abrir %s\n", ficheiro);
        return 0;
    }

    char chave[50];
    while (fscanf(f, "%s", chave) == 1) {
        if      (strcmp(chave, "MAX_ESPERA")                == 0) fscanf(f, "%d", &config->max_espera);
        else if (strcmp(chave, "N_CAIXAS")                  == 0) fscanf(f, "%d", &config->n_caixas);
        else if (strcmp(chave, "TEMPO_ATENDIMENTO_PRODUTO") == 0) fscanf(f, "%d", &config->tempo_atendimento_produto);
        else if (strcmp(chave, "MAX_PRECO")                 == 0) fscanf(f, "%f", &config->max_preco);
        else if (strcmp(chave, "MAX_FILA")                  == 0) fscanf(f, "%d", &config->max_fila);
        else if (strcmp(chave, "MIN_FILA")                  == 0) fscanf(f, "%d", &config->min_fila);
        else if (strcmp(chave, "HORA_ABERTURA")             == 0) fscanf(f, "%d", &config->hora_abertura);
        else if (strcmp(chave, "HORA_FECHO")                == 0) fscanf(f, "%d", &config->hora_fecho);
    }
    fclose(f);
    return 1;
}

/* =====================================
 
 ===================================== */
void lerClientes(const char *ficheiro, HashTable *ht) {
    FILE *f = fopen(ficheiro, "r");
    if (!f) {
        printf("Erro: nao foi possivel abrir %s\n", ficheiro);
        return;
    }

    char linha[256];
    while (fgets(linha, sizeof(linha), f)) {
        if (strlen(linha) <= 1) continue;
        linha[strcspn(linha, "\n")] = '\0';

        char *tab = strchr(linha, '\t');
        if (!tab) continue;
        *tab = '\0';

        int id = atoi(linha);
        char *nome = tab + 1;

        Cliente *c = malloc(sizeof(Cliente));
        if (!c) continue;
        c->id                = id;
        strncpy(c->nome, nome, MAX_NOME - 1);
        c->nome[MAX_NOME - 1] = '\0';
        c->n_produtos         = 0;
        c->carrinho           = NULL;
        c->sim_time_entrada   = -1;
        c->produto_oferecido  = 0;
        c->proximo            = NULL;

        int bucket = id % HASH_SIZE;
        NodoHash *nodo = malloc(sizeof(NodoHash));
        if (!nodo) { free(c); continue; }
        nodo->id_cliente    = id;
        nodo->cliente       = c;
        nodo->indice_caixa  = -1;
        nodo->proximo       = ht->buckets[bucket];
        ht->buckets[bucket] = nodo;
        ht->total_clientes++;
    }

    fclose(f);
}

/* =====================================
 
 ===================================== */
Produto *lerProdutos(const char *ficheiro, int *total, int tempo_max) {
    FILE *f = fopen(ficheiro, "r");
    if (!f) {
        printf("Erro: nao foi possivel abrir %s\n", ficheiro);
        *total = 0;
        return NULL;
    }

    int count = 0;
    char linha[512];
    while (fgets(linha, sizeof(linha), f)) {
        if (strlen(linha) > 1) count++;
    }

    Produto *produtos = malloc(count * sizeof(Produto));
    if (!produtos) {
        fclose(f);
        *total = 0;
        return NULL;
    }

    rewind(f);
    int i = 0;
    while (i < count && fgets(linha, sizeof(linha), f)) {
        if (strlen(linha) <= 1) continue;
        linha[strcspn(linha, "\n")] = '\0';

        char *token = strtok(linha, "\t");
        if (!token) continue;
        produtos[i].id = atoi(token);

        token = strtok(NULL, "\t");
        if (!token) continue;
        strncpy(produtos[i].nome, token, MAX_NOME - 1);
        produtos[i].nome[MAX_NOME - 1] = '\0';

        token = strtok(NULL, "\t");
        if (!token) continue;
        produtos[i].preco = atof(token);

        produtos[i].tempo_passagem = (rand() % (tempo_max - 2 + 1)) + 2;
        produtos[i].proximo = NULL;

        i++;
    }

    fclose(f);
    *total = i;
    return produtos;
}

/* =====================================
 
 ===================================== */
void lerDados(const char *ficheiro, Supermercado *sm) {
    FILE *f = fopen(ficheiro, "r");
    if (!f) {
        printf("Erro: nao foi possivel abrir %s\n", ficheiro);
        return;
    }

    char linha[512];

    /* Ignorar linhas de comentário e ler número de caixas */
    int n_caixas = 0;
    while (fgets(linha, sizeof(linha), f)) {
        if (linha[0] == '/' || strlen(linha) <= 1) continue;
        sscanf(linha, "%d", &n_caixas);
        break;
    }

    for (int i = 0; i < n_caixas; i++) {
        /* Ler linha da caixa: "CaixaN : ESTADO" */
        while (fgets(linha, sizeof(linha), f)) {
            if (linha[0] != '/' && strlen(linha) > 1) break;
        }
        char nome_caixa[MAX_NOME];
        int ativa;
        sscanf(linha, "%s : %d", nome_caixa, &ativa);

        sm->caixas[i].id    = i + 1;
        strncpy(sm->caixas[i].nome, nome_caixa, MAX_NOME - 1);
        sm->caixas[i].ativa = ativa;
        sm->caixas[i].fila.frente  = NULL;
        sm->caixas[i].fila.fim     = NULL;
        sm->caixas[i].fila.tamanho = 0;
        sm->caixas[i].total_clientes_atendidos = 0;
        sm->caixas[i].total_produtos_vendidos  = 0;
        sm->caixas[i].total_valor_vendido      = 0.0f;
        sm->caixas[i].produtos_oferecidos      = 0;
        sm->caixas[i].valor_oferecido          = 0.0f;
        sm->caixas[i].sim_time_fim_atendimento = sm->config.hora_abertura * 3600;

        /* Ler operador: "OPERADOR : NomeOperador" */
        while (fgets(linha, sizeof(linha), f)) {
            if (linha[0] != '/' && strlen(linha) > 1) break;
        }
        char operador_nome[MAX_NOME];
        sscanf(linha, "OPERADOR : %s", operador_nome);
        strncpy(sm->caixas[i].operador_nome, operador_nome, MAX_NOME - 1);

        /* Ler número de clientes */
        while (fgets(linha, sizeof(linha), f)) {
            if (linha[0] != '/' && strlen(linha) > 1) break;
        }
        int n_clientes;
        sscanf(linha, "%d", &n_clientes);

        /* Ler cada cliente */
        for (int j = 0; j < n_clientes; j++) {
            while (fgets(linha, sizeof(linha), f)) {
                if (linha[0] != '/' && strlen(linha) > 1) break;
            }
            linha[strcspn(linha, "\n")] = '\0';

            Cliente *c = malloc(sizeof(Cliente));
            if (!c) continue;
            c->n_produtos        = 0;
            c->carrinho          = NULL;
            c->sim_time_entrada = sm->config.hora_abertura * 3600;
            c->produto_oferecido = 0;
            c->proximo           = NULL;
            c->id                = 0;

            /* Ler "NomeCliente : N_PRODUTOS  PROD1 PROD2 ..." */
            char *token = strtok(linha, " ");
            strncpy(c->nome, token, MAX_NOME - 1);

            strtok(NULL, " ");  /* salta o ":" */

            token = strtok(NULL, " ");
            int n_prod = atoi(token);
            c->n_produtos = n_prod;

            /* Ler produtos do carrinho */
            Produto *ultimo = NULL;
            for (int k = 0; k < n_prod; k++) {
                token = strtok(NULL, " ");
                if (!token) break;

                Produto *p = malloc(sizeof(Produto));
                if (!p) continue;
                strncpy(p->nome, token, MAX_NOME - 1);
                p->nome[MAX_NOME - 1] = '\0';
                p->id             = 0;
                p->preco          = 0.0f;  /* desconhecido nos dados iniciais */
                p->tempo_passagem = (rand() % (sm->config.tempo_atendimento_produto - 2 + 1)) + 2;
                p->proximo        = NULL;

                if (c->carrinho == NULL) {
                    c->carrinho = p;
                } else {
                    ultimo->proximo = p;
                }
                ultimo = p;
            }

            /* Enqueue na caixa */
            if (sm->caixas[i].fila.fim == NULL) {
                sm->caixas[i].fila.frente = c;
                sm->caixas[i].fila.fim    = c;
            } else {
                sm->caixas[i].fila.fim->proximo = c;
                sm->caixas[i].fila.fim          = c;
            }
            sm->caixas[i].fila.tamanho++;
        }
    }

    fclose(f);
}
