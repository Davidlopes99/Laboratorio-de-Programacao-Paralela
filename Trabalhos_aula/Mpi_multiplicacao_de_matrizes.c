#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define N 5

int main(int argc, char *argv[]) {
    int meu_ranque, num_procs, i, j, k;
    int *A = NULL, *B = NULL, *C = NULL, *A_local, *C_local;
    int *sendcounts = NULL, *displs = NULL;
    int linhas_local, base, resto, desloc;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    base = N / num_procs;
    resto = N % num_procs;
    linhas_local = (meu_ranque < resto) ? base + 1 : base;

    A_local = malloc(linhas_local * N * sizeof(int));
    C_local = malloc(linhas_local * N * sizeof(int));
    B = malloc(N * N * sizeof(int));

    if (meu_ranque == 0) {
        A = malloc(N * N * sizeof(int));
        C = malloc(N * N * sizeof(int));
        sendcounts = malloc(num_procs * sizeof(int));
        displs = malloc(num_procs * sizeof(int));
        
        int A_input[N][N] = {
            {1, 2, 3, 4, 5},
            {5, 4, 3, 2, 1},
            {1, 1, 1, 1, 1},
            {2, 3, 4, 5, 6},
            {6, 5, 4, 3, 2}
        };

        int B_input[N][N] = {
            {1, 0, 0, 0, 0},
            {0, 1, 0, 0, 0},
            {0, 0, 1, 0, 0},
            {0, 0, 0, 1, 0},
            {0, 0, 0, 0, 1}
        };

        for (i = 0; i < N; i++)
            for (j = 0; j < N; j++) {
                A[i*N+j] = A_input[i][j];
                B[i*N+j] = B_input[i][j];
            }

        printf("Matriz A:\n");
        for (i = 0; i < N; i++, printf("\n"))
            for (j = 0; j < N; j++)
                printf("%4d ", A[i*N+j]);

        printf("\nMatriz B:\n");
        for (i = 0; i < N; i++, printf("\n"))
            for (j = 0; j < N; j++)
                printf("%4d ", B[i*N+j]);

        for (i = 0, desloc = 0; i < num_procs; i++) {
            int linhas = (i < resto) ? base + 1 : base;
            sendcounts[i] = linhas * N;
            displs[i] = desloc;
            desloc += linhas * N;
        }
    }

    MPI_Scatterv(A, sendcounts, displs, MPI_INT, A_local, linhas_local*N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(B, N*N, MPI_INT, 0, MPI_COMM_WORLD);

    for (i = 0; i < linhas_local; i++)
        for (j = 0; j < N; j++) {
            C_local[i*N+j] = 0;
            for (k = 0; k < N; k++)
                C_local[i*N+j] += A_local[i*N+k] * B[k*N+j];
        }

    MPI_Gatherv(C_local, linhas_local*N, MPI_INT, C, sendcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    if (meu_ranque == 0) {
        printf("\nMatriz Resultado C = A x B:\n");
        for (i = 0; i < N; i++, printf("\n"))
            for (j = 0; j < N; j++)
                printf("%4d ", C[i*N+j]);
    }

    free(A_local); free(B); free(C_local);
    if (meu_ranque == 0) {
        free(A); free(C); free(sendcounts); free(displs);
    }

    MPI_Finalize();
    return 0;
}

//cd ~/lab-paralela
//mpicc Mpi_multiplicacao_de_matrizes.c -o matriz_mpi
//mpiexec -n 4 ./matriz_mpi