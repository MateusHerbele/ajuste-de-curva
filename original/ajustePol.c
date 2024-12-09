#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <fenv.h>
#include <math.h>
#include <stdint.h>
#include <likwid.h>

#include "utils.h"

/////////////////////////////////////////////////////////////////////////////////////
//   AJUSTE DE CURVAS
/////////////////////////////////////////////////////////////////////////////////////

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

void montaSL(double *A, double *b, int n, long long int p, double *x, double *y){
  // Preenchendo a matriz A
  int base = 0;
  int pos = 0;
  for(int i = 0; i < n; ++i){
    for(int j = 0; j < n; j += 5){
      base = i * n + j;
      A[base] = 0.0;
      A[base + 1] = 0.0;
      A[base + 2] = 0.0;
      A[base + 3] = 0.0;
      A[base + 4] = 0.0;
      pos = i + j;
      for(long long int k = 0; k < p; ++k){
        A[base] += int_pow(x[k], pos);
        A[base + 1] += int_pow(x[k], pos + 1);
        A[base + 2] += int_pow(x[k], pos + 2);
        A[base + 3] += int_pow(x[k], pos + 3);
        A[base + 4] += int_pow(x[k], pos + 4);
      }
    }
  }
  // Preenchendo o vetor b
  for(int i = 0; i < n; i += 5){
    b[i] = 0.0;
    b[i + 1] = 0.0;
    b[i + 2] = 0.0;
    b[i + 3] = 0.0;
    b[i + 4] = 0.0;
    for(long long int k = 0; k < p; ++k){
      b[i] += int_pow(x[k], i) * y[k];
      b[i + 1] += int_pow(x[k], i + 1) * y[k];
      b[i + 2] += int_pow(x[k], i + 2) * y[k];
      b[i + 3] += int_pow(x[k], i + 3) * y[k];
      b[i + 4] += int_pow(x[k], i + 4) * y[k];
    }
  }
}


void eliminacaoGauss(double *A, double *b, int n, int *linhaIndex){
  for(int i = 0; i < n; ++i){
    int iMax = i;

    // Encontrar o pivô
    for(int k = i + 1; k < n; ++k){
      if (fabs(A[linhaIndex[k] * n + i]) > fabs(A[linhaIndex[iMax] * n + i])){
        iMax = k;
      }
    }

    // Troca as linhas no índice
    if (iMax != i){
      int temp = linhaIndex[i];
      linhaIndex[i] = linhaIndex[iMax];
      linhaIndex[iMax] = temp;
    }

    // Realiza a eliminação de Gauss
    for(int k = i + 1; k < n; ++k){
      double m = A[linhaIndex[k] * n + i] / A[linhaIndex[i] * n + i];
      A[linhaIndex[k] * n + i] = 0.0;

      for(int j = i + 1; j < n; ++j){
        A[linhaIndex[k] * n + j] -= A[linhaIndex[i] * n + j] * m;
      }
      b[linhaIndex[k]] -= b[linhaIndex[i]] * m;
    }
  }
}

void retrossubs(double *A, double *b, double *x, int n, int *linhaIndex){
  for(int i = n - 1; i >= 0; --i){
    int linha = linhaIndex[i];  // Obtem o índice da linha
    x[i] = b[linha];
    for(int j = i + 1; j < n; ++j){
      x[i] -= A[linha * n + j] * x[j];
    }
    x[i] /= A[linha * n + i];
  }
}

double P(double x, int N, double *alpha){
  double Px = alpha[0];
  for(int i = 1; i <= N; ++i)
    Px += alpha[i]*pow(x,i); 
  
  return Px;
}

int main(){
  LIKWID_MARKER_INIT;
  int N, n;
  long long int K, p;

  scanf("%d %lld", &N, &K);
  p = K;   // quantidade de pontos
  n = N+1; // tamanho do SL (grau N + 1)

  double *x = (double *) malloc(sizeof(double)*p);
  double *y = (double *) malloc(sizeof(double)*p);

  // ler numeros
  for(long long int i = 0; i < p; ++i)
    scanf("%lf %lf", x+i, y+i);
  
  double *A = (double *) malloc(sizeof(double) * n * n);
  int* linhaIndex = (int *) malloc(sizeof(int)*n);
  for(int i = 0; i < n; ++i)
    linhaIndex[i] = i;  // Inicializa o mapeamento original das linhas
  
  double *b = (double *) malloc(sizeof(double)*n);
  double *alpha = (double *) malloc(sizeof(double)*n); // coeficientes ajuste

  // (A) Gera SL
  double tSL = timestamp();
  LIKWID_MARKER_START("SL_MMQ");
  montaSL(A, b, n, p, x, y);
  LIKWID_MARKER_STOP("SL_MMQ");
  tSL = timestamp() - tSL;

  // (B) Resolve SL
  double tEG = timestamp();
  LIKWID_MARKER_START("EG");
  eliminacaoGauss(A, b, n, linhaIndex); 
  retrossubs(A, b, alpha, n, linhaIndex);
  LIKWID_MARKER_STOP("EG"); 
  tEG = timestamp() - tEG;

  // Imprime coeficientes
  printf("Coeficientes: ");
  for(int i = 0; i < n; ++i)
    printf("%1.15e ", alpha[i]);
  puts("");

  // Imprime resíduos
  printf("Resíduos: ");
  for(long long int i = 0; i < p; ++i)
    printf("%1.15e ", fabs(y[i] - P(x[i],N,alpha)) );
  puts("");

  printf("Tempos(K, tSL, tEG):");
  // Imprime os tempos
  printf("%lld %1.10e %1.10e\n", K, tSL, tEG);

  LIKWID_MARKER_CLOSE;
  return 0;
}
