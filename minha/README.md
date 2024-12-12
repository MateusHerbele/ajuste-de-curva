# COMANDOS PRA DPS:
pro de tempo:
./gera_entrada <Kpontos> <GrauPol> | ./ajustePol
pelo topolocy -C 7
outros:
./gera_entrada <Kpontos> <GrauPol> | likwid-perfctr -C 7 -g L3CACHE -m ./ajustePol
./gera_entrada <Kpontos> <GrauPol> | likwid-perfctr -C 7 -g ENERGY -m ./ajustePol
./gera_entrada <Kpontos> <GrauPol> | likwid-perfctr -C 7 -g ENERGY -m ./ajustePol



# TESTES PRINCIPAIS:
desenrolar os loopings
e trocar pra um vetor de structs o x e y

# Arquitetura: 

--------------------------------------------------------------------------------
CPU name:	Intel(R) Core(TM) i5-10300H CPU @ 2.50GHz
CPU type:	Intel Cometlake processor
CPU stepping:	2
********************************************************************************
Hardware Thread Topology
********************************************************************************
Sockets:		1
Cores per socket:	4
Threads per core:	2
--------------------------------------------------------------------------------
HWThread        Thread        Core        Die        Socket        Available
0               0             0           0          0             *                
1               0             1           0          0             *                
2               0             2           0          0             *                
3               0             3           0          0             *                
4               1             0           0          0             *                
5               1             1           0          0             *                
6               1             2           0          0             *                
7               1             3           0          0             *                
--------------------------------------------------------------------------------
Socket 0:		( 0 4 1 5 2 6 3 7 )
--------------------------------------------------------------------------------
********************************************************************************
Cache Topology
********************************************************************************
Level:			1
Size:			32 kB
Type:			Data cache
Associativity:		8
Number of sets:		64
Cache line size:	64
Cache type:		Non Inclusive
Shared by threads:	2
Cache groups:		( 0 4 ) ( 1 5 ) ( 2 6 ) ( 3 7 )
--------------------------------------------------------------------------------
Level:			2
Size:			256 kB
Type:			Unified cache
Associativity:		4
Number of sets:		1024
Cache line size:	64
Cache type:		Non Inclusive
Shared by threads:	2
Cache groups:		( 0 4 ) ( 1 5 ) ( 2 6 ) ( 3 7 )
--------------------------------------------------------------------------------
Level:			3
Size:			8 MB
Type:			Unified cache
Associativity:		16
Number of sets:		8192
Cache line size:	64
Cache type:		Inclusive
Shared by threads:	8
Cache groups:		( 0 4 1 5 2 6 3 7 )
--------------------------------------------------------------------------------
********************************************************************************
NUMA Topology
********************************************************************************
NUMA domains:		1
--------------------------------------------------------------------------------
Domain:			0
Processors:		( 0 4 1 5 2 6 3 7 )
Distances:		10
Free memory:		4032.75 MB
Total memory:		15822.3 MB
--------------------------------------------------------------------------------


********************************************************************************
Graphical Topology
********************************************************************************
Socket 0:
+---------------------------------------------+
| +--------+ +--------+ +--------+ +--------+ |
| |  0 4   | |  1 5   | |  2 6   | |  3 7   | |
| +--------+ +--------+ +--------+ +--------+ |
| +--------+ +--------+ +--------+ +--------+ |
| |  32 kB | |  32 kB | |  32 kB | |  32 kB | |
| +--------+ +--------+ +--------+ +--------+ |
| +--------+ +--------+ +--------+ +--------+ |
| | 256 kB | | 256 kB | | 256 kB | | 256 kB | |
| +--------+ +--------+ +--------+ +--------+ |
| +-----------------------------------------+ |
| |                   8 MB                  | |
| +-----------------------------------------+ |
+---------------------------------------------+


# Primeiro teste feito
Alterar a função pow por 

double int_pow(double base, int exp){
    double result = 1.0;
    while(exp > 0){
        if (exp % 2 == 1){ // Expoente ímpar
            result *= base;
        }
        base *= base; // Quadrado da base
        exp /= 2;
    }
    return result;
}
na montagem do SL montaSl
	A[i][j] += int_pow(x[k], i+j);
    b[i] += int_pow(x[k],i) * y[k];


Tentando otimizar com base nos expoentes serem inteiros sempre

melhores resultados em tempo e e l3 miss rate e l3 miss rate

Testes com otimização de cache: void montaSL(double **A, double *b, int n, long long int p, double *x, double *y){
    // Cache das potências de x[k]
    double **x_powers = (double **)malloc(p * sizeof(double *));
    for(long long int k = 0; k < p; ++k){
        x_powers[k] = (double *)malloc((2 * n) * sizeof(double));
        x_powers[k][0] = 1.0; // x^0 = 1
        for(int e = 1; e < 2 * n; ++e){
            x_powers[k][e] = x_powers[k][e - 1] * x[k];
        }
    }

    // Construção da matriz A
    for(int i = 0; i < n; ++i){
        for(int j = 0; j < n; ++j){
            A[i][j] = 0.0;
            for(long long int k = 0; k < p; ++k){
                A[i][j] += x_powers[k][i + j]; // Usa o cache
            }
        }
    }

    // Construção do vetor b
    for(int i = 0; i < n; ++i){
        b[i] = 0.0;
        for(long long int k = 0; k < p; ++k){
            b[i] += x_powers[k][i] * y[k]; // Usa o cache
        }
    }

    // Liberação da memória do cache
    for(long long int k = 0; k < p; ++k){
        free(x_powers[k]);
    }
    free(x_powers);
}
Não foi satisfatório, resultados com int_pow superiores em todos os aspectos, menos L3 miss ratio que foi levemente menor.

# Matriz pra vetor (mantido)
Teste de subsituição da forma de represetntar a matriz A
no lugar de usar uma matriz clássica, vou alocar tudo em um vetor grande e fazer o acesso como se fosse uma matriz

tentar otimizar o cache do programa
Houve um pequeno aumento nno tempo de execução, quase irrelevante, porém tudo relaciona a cache foi otimizado.

# teste calloc sl (descartado)
substituir malloc por calloc era uma ideia que tive pois na hora da montagem do sl n seria preciso atribuir valors 0.0, porém o miss ratio aumentou, isso se deve que ao atribuir os valores já é puxado na cache e fica em um espaço mais acessível ao processo.

# teste x,y substituir por um array of struct (descartado)
Antes de realizar os testes eu já tenho crtz que não será otimizado nada a partir dele. Pelo código é claro que x e y não aparecem juntos com frequencia para que seja melhor trata-los assim.

    typedef struct{
        double x;
        double y;
    } Po
  Ponto *pontos = (Ponto *) malloc(sizeof(Ponto) * p);

E como esperado, o miss ratio dobrou.

# Unroll

A (mantido): 
levando em consideração que a linha de cache da maquina usada nesse trabalho é 64 bytes e double é representado por 8 bytes, uma cache line cabe 8 doubles, então farei o unroll com base nisso.
Como n só será 101 ou 11, n vale usar algo q não seja um divisor comum de ambos, temos 3 opções, 2, 5 e 10.
2 é muito pequeno para aproveitar o máximo da cache line, 10 é muito grande, passa dela, ent 5 foi escolhido.
Teve um desempenho melhor em relação a acesso a cache.

B (Mantido):
B também tem relação de alocação com n então podemos fazer o mesmo unroll com o valor de 5 usado em A.
Também teve um desempenho melhor em relação ao acesso da cache.

Pequenas otimizações feitas tmb com a variável base e pos para evitar repetição de cálculos. Mesmo que sejam com valores inteiros.

# Inline

Função int pow - > piorou o tempo de execução em 400% (descartado)

# Tentar fazer um mapemamento das linhas da matriz com um vetor auxiliar

Melhorou tanto na perfomance de tempo quanto no request rate da cache