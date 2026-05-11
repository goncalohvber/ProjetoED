#include <stdio.h>
#include "supermercado.h"

void libertarCarrinho(Produto *carrinho) {
    while (carrinho != NULL) {
        Produto *temp = carrinho;
        carrinho = carrinho->proximo;
        free(temp);
    }
}

void libertarFila(Fila *fila) {
    while (fila->frente != NULL) {
        Cliente *temp = fila->frente;
        fila->frente = fila->frente->proximo;
        libertarCarrinho(temp->carrinho);
        free(temp);
    }
    fila->fim     = NULL;
    fila->tamanho = 0;
}

void libertarHashTable(HashTable *ht) {
    for (int i = 0; i < HASH_SIZE; i++) {
        NodoHash *nodo = ht->buckets[i];
        while (nodo != NULL) {
            NodoHash *temp = nodo;
            nodo = nodo->proximo;
            libertarCarrinho(temp->cliente->carrinho);
            free(temp->cliente);
            free(temp);
        }
        ht->buckets[i] = NULL;
    }
    ht->total_clientes = 0;
}

void libertarSupermercado(Supermercado *sm, Produto *produtos) {
    for (int i = 0; i < sm->config.n_caixas; i++)
        libertarFila(&sm->caixas[i].fila);

    libertarHashTable(&sm->clientes);
    free(produtos);
}

