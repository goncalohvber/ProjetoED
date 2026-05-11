#include "supermercado.h"
#include "EstruturasDinamicas.h"

/* ==================================
 Filas
 ================================== */

/*
 O cliente->proximo = NULL no início é importante: garante que o cliente não arrasta um ponteiro inválido de algum estado anterior. Depois há dois casos: se a fila estiver vazia, o cliente é simultaneamente a frente e o fim. Se já tiver clientes, liga-se ao último que existia e passa a ser o novo fim.
 */
void enqueue(Fila *fila, Cliente *cliente) {
    cliente->proximo = NULL;

    if (fila->fim == NULL) {
        fila->frente = cliente;
        fila->fim    = cliente;
    } else {
        fila->fim->proximo = cliente;
        fila->fim          = cliente;
    }
    fila->tamanho++;
}

/*
 
 */
Cliente *dequeue(Fila *fila) {
    if (fila->frente == NULL) return NULL;

    Cliente *removido = fila->frente;
    fila->frente = fila->frente->proximo;

    if (fila->frente == NULL)
        fila->fim = NULL;

    removido->proximo = NULL;
    fila->tamanho--;
    return removido;
}

Cliente *removeFromQueue(Fila *fila, int id_cliente) {
    if (fila->frente == NULL) return NULL;

    Cliente *atual    = fila->frente;
    Cliente *anterior = NULL;

    while (atual != NULL) {
        if (atual->id == id_cliente) {
            if (anterior == NULL) {
                fila->frente = atual->proximo;
            } else {
                anterior->proximo = atual->proximo;
            }

            if (atual->proximo == NULL)
                fila->fim = anterior;

            atual->proximo = NULL;
            fila->tamanho--;
            return atual;
        }
        anterior = atual;
        atual    = atual->proximo;
    }

    return NULL;
}

int queueSize(Fila *fila) {
    return fila->tamanho;
}

/*=========================
 HASH
========================= */
void hashInsert(HashTable *ht, Cliente *cliente) {
    int bucket = cliente->id % HASH_SIZE;

    NodoHash *nodo = malloc(sizeof(NodoHash));
    if (!nodo) return;

    nodo->id_cliente   = cliente->id;
    nodo->cliente      = cliente;
    nodo->indice_caixa = -1;
    nodo->proximo      = ht->buckets[bucket];
    ht->buckets[bucket] = nodo;
    ht->total_clientes++;
}

Cliente *hashSearch(HashTable *ht, int id_cliente) {
    int bucket = id_cliente % HASH_SIZE;
    NodoHash *nodo = ht->buckets[bucket];

    while (nodo != NULL) {
        if (nodo->id_cliente == id_cliente)
            return nodo->cliente;
        nodo = nodo->proximo;
    }

    return NULL;
}

void hashRemove(HashTable *ht, int id_cliente) {
    int bucket = id_cliente % HASH_SIZE;
    NodoHash *atual    = ht->buckets[bucket];
    NodoHash *anterior = NULL;

    while (atual != NULL) {
        if (atual->id_cliente == id_cliente) {
            if (anterior == NULL)
                ht->buckets[bucket] = atual->proximo;
            else
                anterior->proximo = atual->proximo;

            free(atual->cliente);
            free(atual);
            ht->total_clientes--;
            return;
        }
        anterior = atual;
        atual    = atual->proximo;
    }
}

/*=====================
 Caixas
===================== */

void openCaixa(Supermercado *sm, int indice, char *operador_nome, int operador_id) {
    if (indice < 0 || indice >= MAX_CAIXAS) return;
    if (sm->caixas[indice].ativa) return;

    sm->caixas[indice].ativa                      = 1;
    sm->caixas[indice].sim_time_fim_atendimento   = sm->sim_time_atual;
    strncpy(sm->caixas[indice].operador_nome, operador_nome, MAX_NOME - 1);
    sm->caixas[indice].operador_id                = operador_id;
}



void redistributeClients(Supermercado *sm, int indice_origem) {
    if (sm->caixas[indice_origem].fila.tamanho == 0) return;

    while (sm->caixas[indice_origem].fila.tamanho > 0) {
        Cliente *c = dequeue(&sm->caixas[indice_origem].fila);
        if (!c) break;

        /* Encontrar a caixa ativa com menos clientes */
        int destino    = -1;
        int menor_fila = INT_MAX;
        for (int i = 0; i < sm->config.n_caixas; i++) {
            if (i == indice_origem) continue;
            if (!sm->caixas[i].ativa) continue;
            if (sm->caixas[i].fila.tamanho < menor_fila) {
                menor_fila = sm->caixas[i].fila.tamanho;
                destino    = i;
            }
        }

        if (destino == -1) {
            /* Não há caixas abertas — recolocar na origem e parar */
            enqueue(&sm->caixas[indice_origem].fila, c);
            break;
        }

        /* Atualizar tempo de entrada e indice na hash table */
        c->sim_time_entrada = sm->sim_time_atual;
        NodoHash *nodo = sm->clientes.buckets[c->id % HASH_SIZE];
        while (nodo) {
            if (nodo->id_cliente == c->id) {
                nodo->indice_caixa = destino;
                break;
            }
            nodo = nodo->proximo;
        }

        enqueue(&sm->caixas[destino].fila, c);
    }
}

void closeCaixa(Supermercado *sm, int indice) {
    if (indice < 0 || indice >= MAX_CAIXAS) return;
    if (!sm->caixas[indice].ativa) return;

    sm->caixas[indice].ativa = 0;
    redistributeClients(sm, indice);
}
