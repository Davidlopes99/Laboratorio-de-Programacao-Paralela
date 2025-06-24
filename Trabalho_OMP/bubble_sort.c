#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

void bubbleSortParallel(int *arr, int n, int num_threads) {
    int i, j, temp;
    int swapped;

    omp_set_num_threads(num_threads);

    for (i = 0; i < n - 1; i++) {
        swapped = 0;

        #pragma omp parallel for shared(arr, swapped)
        for (j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                // Swap
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;

                swapped = 1;
            }
        }

        if (swapped == 0)
            break;
    }
}

void bubbleSortSequential(int *arr, int n) {
    int i, j, temp;
    for (i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                swapped = 1;
            }
        }
        if (swapped == 0)
            break;
    }
}

void generateRandomArray(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 1000000;
    }
}

int *copyArray(int *source, int n) {
    int *dest = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        dest[i] = source[i];
    }
    return dest;
}

int main() {
    int sizes[] = {50000, 100000, 500000};

    for (int s = 0; s < 3; s++) {
        int n = sizes[s];
        int *array = malloc(n * sizeof(int));
        generateRandomArray(array, n);

        printf("\nLista de %d elementos:\n", n);

        // Sequencial
        int *seq = copyArray(array, n);
        double start = omp_get_wtime();
        bubbleSortSequential(seq, n);
        double end = omp_get_wtime();
        printf("Sequencial: %.4f segundos\n", end - start);
        free(seq);

        // Paralelo 2 threads
        int *par2 = copyArray(array, n);
        start = omp_get_wtime();
        bubbleSortParallel(par2, n, 2);
        end = omp_get_wtime();
        printf("Paralelo 2 threads: %.4f segundos\n", end - start);
        free(par2);

        // Paralelo 4 threads
        int *par4 = copyArray(array, n);
        start = omp_get_wtime();
        bubbleSortParallel(par4, n, 4);
        end = omp_get_wtime();
        printf("Paralelo 4 threads: %.4f segundos\n", end - start);
        free(par4);

        free(array);
    }

    return 0;
}
