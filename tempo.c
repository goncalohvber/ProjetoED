#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "supermercado.h"


int definir_ticks(int abertura, int fecho, int temp, int tick_seg)
{
    int duracao_dia        = fecho - abertura;                  /* segundos simulados num dia */
    int duracao_dia_real_ms = temp * 60 * 1000;                 /* ms reais num dia */
    int num_ticks          = duracao_dia / tick_seg;            /* nº de ticks por dia */
    int ms_por_tick        = duracao_dia_real_ms / num_ticks;   /* ms reais por tick */
    return ms_por_tick;
}


/*
void simular(Supermercado *sm, int ms_por_tick, int tick_seg)
{
    while (sm->sim_time_atual < HORA_FECHO)
    {
        // 1. processar atendimento de cada caixa
        
        // 2. verificar MAX_ESPERA e ofertas
        
        // 3. chegada aleatória de clientes
        
        // 4. abertura/fecho automático de caixas
        
        // avançar o relógio
        sm->sim_time_atual += tick_seg;
        
        // esperar o tempo real correspondente
        Sleep(ms_por_tick); // Windows
        // usleep(ms_por_tick * 1000); // Linux
    }
}
*/

/* ================================================================
   1. PROCESSAR ATENDIMENTO DE CADA CAIXA
   ================================================================ */
void processar_atendimento(Supermercado *sm)
{
    for (int i = 0; i < sm->config.n_caixas; i++)
    {
        if (!sm->caixas[i].ativa) continue;
        if (sm->caixas[i].fila.tamanho == 0) continue;

        if (sm->sim_time_atual >= sm->caixas[i].sim_time_fim_atendimento)
        {
            Cliente *atendido = sm->caixas[i].fila.frente;
            sm->caixas[i].fila.frente = atendido->proximo;
            sm->caixas[i].fila.tamanho--;
            if (sm->caixas[i].fila.tamanho == 0)
                sm->caixas[i].fila.fim = NULL;

            sm->caixas[i].total_clientes_atendidos++;
            sm->caixas[i].total_produtos_vendidos += atendido->n_produtos;

            Produto *p = atendido->carrinho;
            while (p != NULL)
            {
                sm->caixas[i].total_valor_vendido += p->preco;
                Produto *tmp = p;
                p = p->proximo;
                free(tmp);
            }

            free(atendido);

            if (sm->caixas[i].fila.frente != NULL)
            {
                Cliente *proximo = sm->caixas[i].fila.frente;
                int tempo_total = 0;
                Produto *prod = proximo->carrinho;
                while (prod != NULL)
                {
                    tempo_total += prod->tempo_passagem;
                    prod = prod->proximo;
                }
                sm->caixas[i].sim_time_fim_atendimento = sm->sim_time_atual + tempo_total;
            }
        }
    }
}

/* ================================================================
   2. VERIFICAR MAX_ESPERA E OFERTAS
   ================================================================ */
void verificar_ofertas(Supermercado *sm)
{
    for (int i = 0; i < sm->config.n_caixas; i++)
    {
        if (!sm->caixas[i].ativa) continue;

        Cliente *c = sm->caixas[i].fila.frente;
        if (c != NULL) c = c->proximo;

        while (c != NULL)
        {
            double tempo_espera = sm->sim_time_atual - c->sim_time_entrada;

            if (tempo_espera > sm->config.max_espera && !c->produto_oferecido)
            {
                Produto *mais_barato = c->carrinho;
                Produto *p = c->carrinho;
                while (p != NULL)
                {
                    if (p->preco < mais_barato->preco)
                        mais_barato = p;
                    p = p->proximo;
                }

                sm->caixas[i].produtos_oferecidos++;
                sm->caixas[i].valor_oferecido += mais_barato->preco;
                sm->produtos_oferecidos_total++;
                sm->valor_oferecido_total += mais_barato->preco;

                c->produto_oferecido = 1;

                printf("[%.0f s] OFERTA: produto '%s' (%.2f EUR) dado a %s na %s\n",
                       sm->sim_time_atual,
                       mais_barato->nome,
                       mais_barato->preco,
                       c->nome,
                       sm->caixas[i].nome);
            }

            c = c->proximo;
        }
    }
}

/* ================================================================
   3. CHEGADA ALEATÓRIA DE CLIENTES
   ================================================================ */
void chegada_clientes(Supermercado *sm)
{
    if (rand() % 10 >= 3) return; /* ~30% de probabilidade por tick */

    int idx_menor = -1;
    int menor = __INT_MAX__;
    for (int i = 0; i < sm->config.n_caixas; i++)
    {
        if (sm->caixas[i].ativa && sm->caixas[i].fila.tamanho < menor)
        {
            menor = sm->caixas[i].fila.tamanho;
            idx_menor = i;
        }
    }

    if (idx_menor < 0 || menor >= sm->config.max_fila) return;

    Cliente *novo = (Cliente *)malloc(sizeof(Cliente));
    novo->id = rand() % 100000;
    snprintf(novo->nome, MAX_NOME, "Cliente_%d", novo->id);
    novo->sim_time_entrada = sm->sim_time_atual;
    novo->produto_oferecido = 0;
    novo->proximo = NULL;

    int n_prod = rand() % 10 + 1;
    novo->n_produtos = n_prod;
    novo->carrinho = NULL;
    for (int p = 0; p < n_prod; p++)
    {
        Produto *prod = (Produto *)malloc(sizeof(Produto));
        prod->id = rand() % 10000;
        snprintf(prod->nome, MAX_NOME, "Produto_%d", prod->id);
        prod->preco = (float)(rand() % (int)sm->config.max_preco) + 0.5f;
        prod->tempo_passagem = rand() % (sm->config.tempo_atendimento_produto - 2 + 1) + 2;
        prod->proximo = novo->carrinho;
        novo->carrinho = prod;
    }

    if (sm->caixas[idx_menor].fila.fim == NULL)
    {
        sm->caixas[idx_menor].fila.frente = novo;
        sm->caixas[idx_menor].fila.fim = novo;

        int tempo_total = 0;
        Produto *p = novo->carrinho;
        while (p != NULL) { tempo_total += p->tempo_passagem; p = p->proximo; }
        sm->caixas[idx_menor].sim_time_fim_atendimento = sm->sim_time_atual + tempo_total;
    }
    else
    {
        sm->caixas[idx_menor].fila.fim->proximo = novo;
        sm->caixas[idx_menor].fila.fim = novo;
    }
    sm->caixas[idx_menor].fila.tamanho++;
}

/* ================================================================
   4. ABERTURA / FECHO AUTOMÁTICO DE CAIXAS
   ================================================================ */
void gerir_caixas(Supermercado *sm)
{
    int total_clientes = 0;
    int n_ativas = 0;
    for (int i = 0; i < sm->config.n_caixas; i++)
    {
        if (sm->caixas[i].ativa)
        {
            total_clientes += sm->caixas[i].fila.tamanho;
            n_ativas++;
        }
    }

    if (n_ativas == 0) return;

    float media = (float)total_clientes / n_ativas;

    if (media > sm->config.max_fila)
    {
        for (int i = 0; i < sm->config.n_caixas; i++)
        {
            if (!sm->caixas[i].ativa)
            {
                sm->caixas[i].ativa = 1;
                printf("[%.0f s] ABERTURA: %s aberta (media=%.1f > max_fila=%d)\n",
                       sm->sim_time_atual, sm->caixas[i].nome,
                       media, sm->config.max_fila);
                break;
            }
        }
    }

    if (media < sm->config.min_fila && n_ativas > 1)
    {
        int idx_fechar = -1;
        int menor = __INT_MAX__;
        for (int i = 0; i < sm->config.n_caixas; i++)
        {
            if (sm->caixas[i].ativa && sm->caixas[i].fila.tamanho < menor)
            {
                menor = sm->caixas[i].fila.tamanho;
                idx_fechar = i;
            }
        }

        if (idx_fechar >= 0)
        {
            Cliente *c = sm->caixas[idx_fechar].fila.frente;
            while (c != NULL)
            {
                Cliente *proximo = c->proximo;
                c->proximo = NULL;

                int idx_destino = -1;
                int min_dest = __INT_MAX__;
                for (int i = 0; i < sm->config.n_caixas; i++)
                {
                    if (i != idx_fechar && sm->caixas[i].ativa &&
                        sm->caixas[i].fila.tamanho < min_dest)
                    {
                        min_dest = sm->caixas[i].fila.tamanho;
                        idx_destino = i;
                    }
                }

                if (idx_destino >= 0)
                {
                    c->sim_time_entrada = sm->sim_time_atual;
                    c->produto_oferecido = 0;

                    if (sm->caixas[idx_destino].fila.fim == NULL)
                    {
                        sm->caixas[idx_destino].fila.frente = c;
                        sm->caixas[idx_destino].fila.fim = c;
                    }
                    else
                    {
                        sm->caixas[idx_destino].fila.fim->proximo = c;
                        sm->caixas[idx_destino].fila.fim = c;
                    }
                    sm->caixas[idx_destino].fila.tamanho++;
                }
                c = proximo;
            }

            sm->caixas[idx_fechar].ativa = 0;
            sm->caixas[idx_fechar].fila.frente = NULL;
            sm->caixas[idx_fechar].fila.fim = NULL;
            sm->caixas[idx_fechar].fila.tamanho = 0;

            printf("[%.0f s] FECHO: %s fechada (media=%.1f < min_fila=%d)\n",
                   sm->sim_time_atual, sm->caixas[idx_fechar].nome,
                   media, sm->config.max_fila);
        }
    }
}

/* ================================================================
   LOOP PRINCIPAL
   ================================================================ */
void simular(Supermercado *sm, int ms_por_tick, int tick_seg)
{
    while (sm->sim_time_atual < HORA_FECHO)
    {
        processar_atendimento(sm);
        verificar_ofertas(sm);
        chegada_clientes(sm);
        gerir_caixas(sm);

        sm->sim_time_atual += tick_seg;
        Sleep(ms_por_tick);
    }
}
