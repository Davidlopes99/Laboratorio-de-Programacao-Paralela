#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Uso: %s <num_threads> <tamanho_N>\n", argv[0]);
        return 1;
    }

    int nthreads = atoi(argv[1]);
    int N = atoi(argv[2]);
    double dot = 0.0;
    long int i;

    int *a = (int *) malloc(sizeof(int) * N);
    int *b = (int *) malloc(sizeof(int) * N);

    for (i = 0; i < N; i++) {
        a[i] = 2;
        b[i] = 5;
    }

    omp_set_num_threads(nthreads);
    clock_t start, end;
    start = clock();

    // Parallel For + SIMD com redução
    #pragma omp parallel for simd reduction(+:dot)
    for (i = 0; i < N; i++) {
        dot += a[i] * b[i];
    }

    end = clock();
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Resultado: dot = %9.3f com tempo de %f segundos\n", dot, time_taken);

    free(a);
    free(b);
    return 0;
}
