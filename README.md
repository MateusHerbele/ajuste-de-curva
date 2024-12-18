# ajuste-de-curva
EP 03 Computação Científica 

# Arquitetura: 
```
--------------------------------------------------------------------------------
CPU name:       Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz
CPU type:       Intel Skylake processor
CPU stepping:   3
********************************************************************************
Hardware Thread Topology
********************************************************************************
Sockets:                1
CPU dies:               1
Cores per socket:       4
Threads per core:       2
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
Socket 0:               ( 0 4 1 5 2 6 3 7 )
--------------------------------------------------------------------------------
********************************************************************************
Cache Topology
********************************************************************************
Level:                  1
Size:                   32 kB
Type:                   Data cache
Associativity:          8
Number of sets:         64
Cache line size:        64
Cache type:             Non Inclusive
Shared by threads:      2
Cache groups:           ( 0 4 ) ( 1 5 ) ( 2 6 ) ( 3 7 )
--------------------------------------------------------------------------------
Level:                  2
Size:                   256 kB
Type:                   Unified cache
Associativity:          4
Number of sets:         1024
Cache line size:        64
Cache type:             Non Inclusive
Shared by threads:      2
Cache groups:           ( 0 4 ) ( 1 5 ) ( 2 6 ) ( 3 7 )
--------------------------------------------------------------------------------
Level:                  3
Size:                   8 MB
Type:                   Unified cache
Associativity:          16
Number of sets:         8192
Cache line size:        64
Cache type:             Inclusive
Shared by threads:      8
Cache groups:           ( 0 4 1 5 2 6 3 7 )
--------------------------------------------------------------------------------
********************************************************************************
NUMA Topology
********************************************************************************
NUMA domains:           1
--------------------------------------------------------------------------------
Domain:                 0
Processors:             ( 0 4 1 5 2 6 3 7 )
Distances:              10
Free memory:            1072.12 MB
Total memory:           15665.8 MB
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
```
## Linha do tempo das tomadas de decisão ao longo do trabalho:

# Primeiro teste feito
Alterar a função `pow` por:

```c
double int_pow(double base, int exp){
    double result = 1.0;
    while (exp > 0) {
        if (exp % 2 == 1) { // Expoente ímpar
            result *= base;
        }
        base *= base; // Quadrado da base
        exp /= 2;
    }
    return result;
}
```

Na montagem do `montaSL`:

```c
A[i][j] += int_pow(x[k], i + j);
b[i] += int_pow(x[k], i) * y[k];
```

**Tentando otimizar com base nos expoentes serem inteiros sempre.**

- Melhores resultados em tempo e em L3 miss rate.

# Testes com otimização de cache
Função `montaSL` armazenando os cálculos em uma matriz:

```c
void montaSL(double **A, double *b, int n, long long int p, double *x, double *y) {
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
            for (long long int k = 0; k < p; ++k){
                A[i][j] += x_powers[k][i + j]; // Usa o cache
            }
        }
    }

    // Construção do vetor b
    for(int i = 0; i < n; ++i){
        b[i] = 0.0;
        for (long long int k = 0; k < p; ++k){
            b[i] += x_powers[k][i] * y[k]; // Usa o cache
        }
    }

    // Liberação da memória do cache
    for(long long int k = 0; k < p; ++k){
        free(x_powers[k]);
    }
    free(x_powers);
}
```

**Resultados:** Não foi satisfatório. Os resultados com `int_pow` foram superiores em todos os aspectos, menos no L3 miss ratio, que foi levemente menor.

# Matriz para vetor (mantido)
Teste de substituição da forma de representar a matriz `A`:

- No lugar de usar uma matriz clássica, aloquei tudo em um vetor grande e fiz o acesso como se fosse uma matriz.
- Pequeno aumento no tempo de execução (quase irrelevante), mas tudo relacionado a cache foi otimizado.

# Teste calloc no SL (descartado)
Substituir `malloc` por `calloc` parecia uma boa ideia, já que na hora da montagem do SL não seria preciso atribuir valores 0.0 manualmente.

**Resultado:** O miss ratio aumentou. Isso acontece porque atribuir os valores manualmente já os puxa para a cache, deixando-os em um espaço mais acessível.

# Teste x, y substituídos por um array de structs (descartado)

Antes de realizar os testes, já imaginava que não ia dar certo. Pelo código, é claro que `x` e `y` não aparecem juntos com frequência para justificar agrupá-los.

```c
typedef struct {
    double x;
    double y;
} Ponto;

Ponto *pontos = (Ponto *)malloc(sizeof(Ponto) * p);
```

**Resultado:** O miss ratio dobrou.

# Unroll

## A (mantido):
Levei em consideração que a linha de cache da máquina usada nesse trabalho tem 64 bytes e `double` ocupa 8 bytes. Assim, uma cache line comporta 8 `doubles`.

- Como `n` só pode ser 101 ou 11, optei por um divisor comum: 2, 5 ou 10.
- **Escolha:** 5 (2 é pequeno, e 10 ultrapassa a cache line).
- Melhorou o desempenho em relação ao acesso à cache.

## B (mantido):
Mesmo raciocínio aplicado ao vetor `b`.

- **Resultado:** Melhor desempenho no acesso à cache.

**Pequenas otimizações**: Usei variáveis baseadas em índices para evitar repetição de cálculos (mesmo com valores inteiros).

# Inline
Tentei inlining na função `int_pow`.

**Resultado:** Piorou o tempo de execução em 400%. (Descartado)

# Mapeamento das linhas da matriz com vetor auxiliar (mantido)

- Melhorou tanto na performance de tempo quanto no request rate da cache.

# Adicionar restrict nos parâmetros dos ponteiros (mantido)

- Super eficiente! Aumentou o AVX DP em aproximadamente 80% comparado à versão anterior, usando testes reduzidos.

# Remoção dos unrolls para aproveitar uma linha inteira (mantido)

A ideia inicial do unroll era aproveitar mais da linha de cache. Mas com o `restrict`, o compilador já faz essa otimização.

- Removi os unrolls e refiz testes.
- **Resultado:** Aumento significativo no AVX DP.

# Unroll and jam (descartado)
Tentei readicionar unrolls acessando diferentes linhas para carregar várias na cache e otimizar a performance.

**Resultado:**

- Reduziu o AVX DP.
- Aumentou o tempo de execução.
- Reduziu o número de flops por segundo.
- O compilador com `restrict` otimizou melhor que manualmente. Portanto, descartado.

# Alterações que se mantiveram:

- **Restrict nos parâmetros das funções que são ponteiros**, que teve um desempenho melhor dos unrolls feitos pelo compilador do que os meus.
- **"A" deixando de ser uma matriz e virando um vetor que imita o comportamento de uma matriz**, junto da criação do vetor de indexação:
    ```c
    double *A = (double *) malloc(sizeof(double) * n * n);
    int* linhaIndex = (int *) malloc(sizeof(int) * n);
    for (int i = 0; i < n; ++i)
        linhaIndex[i] = i;  // Inicializa o mapeamento original das linhas
    ```
- A aplicação do vetor de indexação em **EG** e **retrossubs**.

---
## Para gerar os gráficos:
 ```bash
    python3 gera_graficos.py
 ```
## Comparação com os gráficos:

### Gráficos na pasta TEMPO:

- **SL**: Como eu acreditava, as otimizações feitas no SL seriam mais efetivas nesse caso. Todas as execuções com o código otimizado tiveram uma performance de tempo melhor aqui, muito provavelmente por conta dos unrolls feitos pelo compilador em detrimento dos restricts.
  
- **EG**: Aqui aconteceram situações divergentes. Para os valores de N2, o código otimizado se mostrou levemente melhor que o código original em todos os valores de K, mas em relação a N1, enquanto o código otimizado começou melhor, ele eventualmente ficou levemente pior que o código original.

### Gráficos na pasta ENERGY:

- Aqui, infelizmente, houve um erro nos cálculos do **likwid** para v2, que ficou armazenado nos resultados para o Professor observar. Isso aconteceu com os valores de K = 10⁵, que deram um valor incrivelmente desbalanceado em relação à continuidade dos testes. Porém, observando o comportamento, acredito que para N2 a V2 continuaria com sua vantagem em relação à versão não otimizada. Em relação a N1, não houve diferença expressiva entre as duas versões. Acredito que a otimização de A, de uma matriz em C para um vetor que representa uma matriz, tem muito a ver com isso, pois para o processador é muito mais vantajoso não ter que administrar uma matriz gigantesca. O vetor sequencialmente grande tem muito menos abstração de endereços, permitindo melhor administração da memória principal e das caches, gerando uma economia de energia mais expressiva para esses valores maiores.

### Gráficos na pasta L3CACHE:

- Aqui houve o mesmo problema com v2 para N2 em K = 10⁵, mas no lugar de um valor absurdamente alto, o valor acabou sendo zerado. Nesse caso, acredito ser mais difícil prever o comportamento do código, pois no caso anterior v2 ficou levemente pior que v1 em EG e um pouco melhor em SL.

- **SL**: Para N1, o código otimizado foi constantemente melhor em relação ao miss ratio, até começar as execuções com K absurdamente altos, quando perdeu severamente para v1. Já para N2, o comportamento dos códigos tende mais para v1. Acredito que isso se deve aos strides feitos por unrolls, que permitiram ao compilador otimizar a permanência de valores de diversas linhas nas caches para mais pontos. É evidente a possibilidade de unroll nessa parte do código, algo que eu mesmo fiz de duas formas diferentes e que deixei registrado na linha do tempo.

- **EG**: N1 afeta os códigos em alternância, mas houve um menor miss ratio em relação a v2. O mesmo ocorreu para N2, onde v2 teve apenas um pico, que acredito ter ocorrido por outras influências, como o uso de outros processos no mesmo núcleo. Porém, esse pico foi menor, o que indica que as otimizações, como o index e a transformação de A para um vetor, além do restrict, que possibilitou ao compilador realizar unrolls no código (mesmo que eu acredite que não tenha ocorrido dado a natureza do código), ajudaram na otimização do uso da cache.

### Gráficos na pasta FLOPS:

- **SL**: Essa parte foi uma surpresa para minha análise, pois v2 teve sempre por volta de metade dos FLOPS DP de v1. Acredito que isso tenha ocorrido devido à substituição da função `pow` pela `int_pow`, mas não posso afirmar com certeza. Para mim, isso faria sentido, pois v2 teve uma execução de tempo muito próxima ou constantemente menor que v1 nos casos, como abordado na parte de tempo. Essa é a minha aposta: o código otimizado fez menos FLOPS/S, mas isso foi uma otimização, e não uma questão de ser menos eficiente. Já o AVX deu zero em tudo, então não foi possível utilizar essa característica.

- **EG**: V2 se manteve constantemente melhor em AVX e FLOPS DP para N2. Para N1, houve uma inconstância em relação a v1, mas o próprio v2 se manteve constante. Então, para valores de N2, o v2 foi bem mais eficiente em relação ao uso dos registradores AVX e com FLOPS DP. Isso provavelmente está relacionado à economia de tempo feita na troca de linha pelo vetor de indexação e pela alteração do formato de A para o vetor que representa uma matriz.

---

## Conclusão final:

Como visto na linha do tempo, acredito que com testes pequenos me precipitei em algumas coisas que afirmei como melhores ou piores, pois para os testes maiores a impermanência do que eu propus veio em alguns casos. Ao mesmo tempo, vejo que o comportamento da otimização foi, no geral, satisfatório. Conseguiu desempenho interessante em tempo para N2 e abriu brechas para uma análise bacana da função `int_pow` e sua influência no número de FLOPS. Admito que o erro no meio do percurso afetou a análise da energia e do cache, mas ainda assim podemos ver o comportamento do restrict no cache e da economia de energia, mesmo que mínima, na versão otimizada.

---

## Anotações finais:

Nunca pensei que teria tanta dor de cabeça fazendo gráficos com o Python. O programa dava muitos problemas devido ao tamanho dos arquivos, por isso fiz o script para a remoção do valor dos coeficientes e dos resíduos. Tive problemas com o posicionamento das legendas, em conseguir encontrar as linhas certas para pegar os valores, e com ambiguidades que não acabavam mais. Foi necessária muita gambiarra para conseguir fazer funcionar de um modo interessante, mas ainda assim acredito que a barreira desse conhecimento possa ter atrapalhado os meus instrumentos de análise.
Outro ponto, foi que eu utilizei o nome PADRAO para os testes de tempo, pois na minha cabeça era um teste sem likwid padrao, reconheço que não é a melhor terminologia, mas por isso esse termo aparece dentro da pasta de tempo e no nome dos arquivos gerados para fazer os gráficos.
