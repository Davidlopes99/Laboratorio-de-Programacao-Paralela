#include <stdio.h>
#include <math.h>
#include "mpi.h"

double f(double x) {
    return x * x; 
}

int main(int argc, char *argv[]) {
    int meu_ranque, num_procs;
    double a = 0.0, b = 1.0;  
    long int n = 500000000;  
    double h, local_a, local_b;
    long int local_n;
    double x, integral = 0.0, total;
    double tempo_inicial, tempo_final;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    h = (b - a) / n;               
    local_n = n / num_procs;       
    local_a = a + meu_ranque * local_n * h;
    local_b = local_a + local_n * h;

    tempo_inicial = MPI_Wtime();

    integral = (f(local_a) + f(local_b)) / 2.0;
    for (long int i = 1; i < local_n; i++) {
        x = local_a + i * h;
        integral += f(x);
    }
    integral *= h;

    MPI_Reduce(&integral, &total, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    tempo_final = MPI_Wtime();

    if (meu_ranque == 0) {
        printf("Foram gastos %3.5f segundos\n", tempo_final - tempo_inicial);
        printf("Com n = %ld trapezoides, a estimativa\n", n);
        printf("da integral de %lf até %lf = %lf\n", a, b, total);
    }

    MPI_Finalize();
    return 0;
}
