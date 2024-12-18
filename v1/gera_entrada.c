#include <stdio.h>
#include <stdlib.h>

#define RUIDO (((double)rand())/(K*(double)RAND_MAX))
#define PASSO (1.0/(double)K)

double f(double x, int N, long long int K, double *a) {
  double fx = a[0] + RUIDO;
  double px = 1;
  for (int i = 1; i <= N; ++i) {
    px *= x;
    fx += a[i]*px;
  }
  return fx;
}

int main(int argc, char **argv) {

  if (argc != 3) {
    printf("uso: %s <K> <N>, onde <K> é a quantidade de pontos e <N> é o grau do polinômio\n", argv[0]);
    return 1;
  }

  long long int K = atoll(argv[1]);
  int N = atoi(argv[2]);

  srand(20242);

  double *a = (double *)malloc(sizeof(double)*(N+1)); // N+1 coeficientes
  a[0] = 0.1;
  for (int i = 1; i <= N; ++i)
    a[i] = a[i-1]/2 + RUIDO;

  printf("%d\n", N);
  printf("%lld\n", K);
  double x = RUIDO;
  for (long long int i = 0; i < K; ++i) {
    printf("%1.15e %1.15e\n", x, f(x,N,K,a));
    x += PASSO;
  }

#ifdef DEBUG
  for (int i = 0; i <= N; ++i)
    fprintf(stderr, "a%d = %1.15e\n", i, a[i]);
#endif
}
