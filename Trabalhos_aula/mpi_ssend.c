#include <stdio.h> 
#include <stdlib.h>
#include "mpi.h"

int main(int argc, char *argv[]) {
    int i, meu_ranque, num_procs;
    int pot2, destino, meu_valor;
    int reducao, recebido, etiq = 1, cont = 1;
    MPI_Status estado;
    MPI_Request requisicao;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // Verifica se o número de processos é potência de 2
    pot2 = num_procs;
    while (((pot2 % 2) == 0) && pot2 > 1)
        pot2 /= 2;
    if (pot2 != 1) {
        if (meu_ranque == 0)
            printf("Por favor execute com número de processos potência de 2.\n");
        MPI_Finalize();
        exit(0);
    }

    meu_valor = meu_ranque * num_procs;
    reducao = meu_valor;

    for (i = 1; i <= (num_procs / 2); i += i) {
        if ((meu_ranque / i) % 2 == 0) {
            destino = meu_ranque + i;

            // Recebe de forma não bloqueante
            MPI_Irecv(&recebido, cont, MPI_INT, destino, etiq, MPI_COMM_WORLD, &requisicao);
            // Envia normalmente
            MPI_Ssend(&reducao, cont, MPI_INT, destino, etiq, MPI_COMM_WORLD);
            // Espera o recebimento finalizar
            MPI_Wait(&requisicao, &estado);

            if (recebido > reducao)
                reducao = recebido;
        } else {
            destino = meu_ranque - i;

            // Recebe de forma não bloqueante
            MPI_Irecv(&recebido, cont, MPI_INT, destino, etiq, MPI_COMM_WORLD, &requisicao);
            // Envia normalmente
            MPI_Ssend(&reducao, cont, MPI_INT, destino, etiq, MPI_COMM_WORLD);
            // Espera o recebimento finalizar
            MPI_Wait(&requisicao, &estado);

            if (recebido > reducao)
                reducao = recebido;
        }
    }

    printf("Meu valor = %d, redução = %d \n", meu_valor, reducao);
    MPI_Finalize();
    return 0;
}


//cd ~/lab-paralela
//mpicc mpi_ssend.c -o mpi_ssend
//mpiexec -n 4 ./mpi_ssend