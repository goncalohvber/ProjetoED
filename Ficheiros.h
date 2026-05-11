#ifndef Ficheiros.h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "supermercado.h"


/*Esta função lê a configuração. Ela primeiro abre o ficheiro e blablabla verifica se ele 
existe... Bom, ela faz scan das palavras no ficheiro dentro do ciclo while, quando 
chave = Max_Espera, ou o que andamos a procurar, ela lê o que está na frente do maximo de 
espera (que vai ser o máximo de espera HAHAAHAH) e é isso. ;)*/
int lerConfiguracao(const char *ficheiro, Configuracao *config);


/*Este aqui fiquei eu a entender como exatamente funcionava durante algum tempo, mas o claudio 
soube explicar. vou explicar-vos +/- nos comentarios*/
// VERIFICAR SE USAMOS FUNCIONARIOS
Funcionario *lerFuncionarios(const char *ficheiro, int *total);


#endif
