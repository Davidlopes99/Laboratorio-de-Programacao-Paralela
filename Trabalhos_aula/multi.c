#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

int main(int argc, char **argv) {
    int N;
    long int i;
    double dot = 0.0;

    /* O número de elementos é o primeiro parâmetro de entrada */
    if (argc < 2) {
        printf("Uso: %s <N>\n", argv[0]);
        return 1;
    }
    N = atoi(argv[1]);

    int* a = (int*) malloc(sizeof(int) * N);
    int* b = (int*) malloc(sizeof(int) * N);

    /* Inicializando os vetores */
    for (i = 0; i < N; i++) {
        a[i] = 2;
        b[i] = 5;
    }

    clock_t start, end;
    start = clock();

    /* Multiplicação - Produto interno com parallel for simd */
    #pragma omp parallel for simd reduction(+:dot)
    for (i = 0; i < N; i++) {
        dot += a[i] * b[i];
    }

    end = clock();
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    free(a);
    free(b);

    printf("Resultado: dot = %9.3f com tempo de %f segundos\n", dot, time_taken);

    return 0;
}
