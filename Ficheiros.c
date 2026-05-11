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
    }
    fclose(f);
    return 1;
}

/* ================================================================
   Este aqui fiquei eu a entender como exatamente funcionava durante algum tempo, mas o claudio soube explicar. vou explicar-vos +/- nos comentarios
   ================================================================ */
Funcionario *lerFuncionarios(const char *ficheiro, int *total) {
    FILE *f = fopen(ficheiro, "r");
    if (!f) {
        printf("Erro: nao foi possivel abrir %s\n", ficheiro);
        *total = 0;
        return NULL;
    }

    int count = 0;
    char linha[256];
    while (fgets(linha, sizeof(linha), f)) {
        if (strlen(linha) > 1) count++; //Serve para ver o numero de linhas, ou seja o numero de funcionarios
    }

    Funcionario *funcionarios = malloc(count * sizeof(Funcionario));
    if (!funcionarios) {
        fclose(f);
        *total = 0;
        return NULL;
    }

    rewind(f); // Volta ao Inicio do ficheiro (todos os dias o homem aprender coisas novas não é verdade?)
    int i = 0;
    while (i < count && fgets(linha, sizeof(linha), f)) { // Aqui o fgets vai ler uma linha, e o sizeof diz lhe o tamanho máximo que tem. Isto vai dar NULL quando chegar ao fim do ficheiro, por isso sai do while no fim.
        if (strlen(linha) <= 1) continue; //Isto tá aqui para nos proteger de linhas vazias. tipo, ele ve o tamanho da linha e se a linha não tiver nd, em vez de parar tudo, ele simplesmente salta este ciclo, e nao faz nd do que ta embaixo.
        linha[strcspn(linha, "\n")] = '\0';
        char *tab = strchr(linha, '\t');
        if (!tab) continue;
        *tab = '\0';
        funcionarios[i].id = atoi(linha);
        strncpy(funcionarios[i].nome, tab + 1, MAX_NOME - 1);
        funcionarios[i].nome[MAX_NOME - 1] = '\0';
        i++;
    }

    fclose(f);
    *total = i;
    return funcionarios;
}

