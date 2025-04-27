#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define N 5 // Você pode mudar o tamanho da matriz aqui

int main(int argc, char *argv[]) {
    int meu_ranque, num_procs;
    int i, j, k;
    int *A = NULL, *B = NULL, *C = NULL;
    int *A_local = NULL, *C_local = NULL;
    int linhas_local;
    int *sendcounts = NULL, *displs = NULL;
    int *recvcounts = NULL, *rdispls = NULL;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    MPI_Comm com = MPI_COMM_WORLD;
    int raiz = 0;

    // Cálculo do número de linhas para cada processo
    int base = N / num_procs;
    int resto = N % num_procs;

    if (meu_ranque < resto)
        linhas_local = base + 1;
    else
        linhas_local = base;

    // Aloca espaço para B e para partes locais de A e C
    A_local = (int *)malloc(linhas_local * N * sizeof(int));
    C_local = (int *)malloc(linhas_local * N * sizeof(int));
    B = (int *)malloc(N * N * sizeof(int));

    if (meu_ranque == raiz) {
        A = (int *)malloc(N * N * sizeof(int));
        C = (int *)malloc(N * N * sizeof(int));

        // Inicializa matriz A
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                A[i*N + j] = i + j; // Exemplo: A[i][j] = i + j
            }
        }

        // Inicializa matriz B
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                B[i*N + j] = i * j; // Exemplo: B[i][j] = i * j
            }
        }

        // Imprime as matrizes A e B
        printf("Matriz A:\n");
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                printf("%4d ", A[i*N + j]);
            }
            printf("\n");
        }
        printf("\nMatriz B:\n");
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                printf("%4d ", B[i*N + j]);
            }
            printf("\n");
        }
        printf("\n");
        
        // Prepara sendcounts e displacements para Scatterv
        sendcounts = (int *)malloc(num_procs * sizeof(int));
        displs = (int *)malloc(num_procs * sizeof(int));
        recvcounts = (int *)malloc(num_procs * sizeof(int));
        rdispls = (int *)malloc(num_procs * sizeof(int));

        int desloc = 0;
        for (i = 0; i < num_procs; i++) {
            int linhas = (i < resto) ? base + 1 : base;
            sendcounts[i] = linhas * N;
            displs[i] = desloc;
            recvcounts[i] = linhas * N;
            rdispls[i] = desloc;
            desloc += linhas * N;
        }
    }

    // Espalha A
    MPI_Scatterv(A, sendcounts, displs, MPI_INT,
                 A_local, linhas_local*N, MPI_INT, raiz, com);

    // Todos os processos recebem B
    MPI_Bcast(B, N*N, MPI_INT, raiz, com);

    // Cada processo calcula sua parte de C
    for (i = 0; i < linhas_local; i++) {
        for (j = 0; j < N; j++) {
            C_local[i*N + j] = 0;
            for (k = 0; k < N; k++) {
                C_local[i*N + j] += A_local[i*N + k] * B[k*N + j];
            }
        }
    }

    // Reúne C
    MPI_Gatherv(C_local, linhas_local*N, MPI_INT,
                C, recvcounts, rdispls, MPI_INT, raiz, com);

    // Processo raiz imprime o resultado
    if (meu_ranque == raiz) {
        printf("Matriz Resultado C = A x B:\n");
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                printf("%4d ", C[i*N + j]);
            }
            printf("\n");
        }
    }

    // Libera memória
    free(A_local);
    free(B);
    free(C_local);
    if (meu_ranque == raiz) {
        free(A);
        free(C);
        free(sendcounts);
        free(displs);
        free(recvcounts);
        free(rdispls);
    }

    MPI_Finalize();
    return 0;
}
