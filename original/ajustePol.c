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

void montaSL(double **A, double *b, int n, long long int p, double *x, double *y) {
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j) {
      A[i][j] = 0.0;
      for (long long int k = 0; k < p; ++k) {
	A[i][j] += pow(x[k], i+j);
      }
    }

  for (int i = 0; i < n; ++i) {
    b[i] = 0.0;
    for (long long int k = 0; k < p; ++k)
      b[i] += pow(x[k],i) * y[k];
  }
}

void eliminacaoGauss(double **A, double *b, int n) {
  for (int i = 0; i < n; ++i) {
    int iMax = i;
    for (int k = i+1; k < n; ++k)
      if (A[k][i] > A[iMax][i])
	iMax = k;
    if (iMax != i) {
      double *tmp, aux;
      tmp = A[i];
      A[i] = A[iMax];
      A[iMax] = tmp;

      aux = b[i];
      b[i] = b[iMax];
      b[iMax] = aux;
    }

    for (int k = i+1; k < n; ++k) {
      double m = A[k][i] / A[i][i];
      A[k][i]  = 0.0;
      for (int j = i+1; j < n; ++j)
	A[k][j] -= A[i][j]*m;
      b[k] -= b[i]*m;
    }
  }
}

void retrossubs(double **A, double *b, double *x, int n) {
  for (int i = n-1; i >= 0; --i) {
    x[i] = b[i];
    for (int j = i+1; j < n; ++j)
      x[i] -= A[i][j]*x[j];
    x[i] /= A[i][i];
  }
}

double P(double x, int N, double *alpha) {
  double Px = alpha[0];
  for (int i = 1; i <= N; ++i)
    Px += alpha[i]*pow(x,i);
  
  return Px;
}

int main() {
  LIKWID_MARKER_INIT;
  int N, n;
  long long int K, p;

  scanf("%d %lld", &N, &K);
  p = K;   // quantidade de pontos
  n = N+1; // tamanho do SL (grau N + 1)

  double *x = (double *) malloc(sizeof(double)*p);
  double *y = (double *) malloc(sizeof(double)*p);

  // ler numeros
  for (long long int i = 0; i < p; ++i)
    scanf("%lf %lf", x+i, y+i);

  double **A = (double **) malloc(sizeof(double *)*n);
  for (int i = 0; i < n; ++i)
    A[i] = (double *) malloc(sizeof(double)*n);
  
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
  eliminacaoGauss(A, b, n); 
  retrossubs(A, b, alpha, n);
  LIKWID_MARKER_STOP("EG"); 
  tEG = timestamp() - tEG;

  // Imprime coeficientes
  printf("Coeficientes: ");
  for (int i = 0; i < n; ++i)
    printf("%1.15e ", alpha[i]);
  puts("");

  // Imprime resíduos
  printf("Resíduos: ");
  for (long long int i = 0; i < p; ++i)
    printf("%1.15e ", fabs(y[i] - P(x[i],N,alpha)) );
  puts("");

  printf("Tempos(K, tSL, tEG):");
  // Imprime os tempos
  printf("%lld %1.10e %1.10e\n", K, tSL, tEG);

  LIKWID_MARKER_CLOSE;
  return 0;
}
