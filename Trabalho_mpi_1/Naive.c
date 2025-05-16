#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

// Função para verificar se um número é primo
int primo(long int n) {
    int i;
    
    // Números pares já são descartados no loop principal
    // Aqui verificamos apenas divisibilidade por ímpares
    for (i = 3; i < (int)(sqrt(n) + 1); i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    double t_inicial, t_final;
    int meu_ranque, num_procs;
    long int n, i;
    int cont = 0, total = 0;
    int inicio, salto;
    int metodo_comunicacao = 1; // Valor padrão
    MPI_Status status;
    MPI_Request request;
    
    // Inicializar MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    
    // Verificar argumentos da linha de comando
    if (argc < 2) {
        if (meu_ranque == 0) {
            printf("Uso: %s <valor_maximo> [metodo_comunicacao]\n", argv[0]);
            printf("metodo_comunicacao (1-10):\n");
            printf("1: MPI_Send + MPI_Recv\n");
            printf("2: MPI_Isend + MPI_Recv\n");
            printf("3: MPI_Rsend + MPI_Recv\n");
            printf("4: MPI_Bsend + MPI_Recv\n");
            printf("5: MPI_Ssend + MPI_Recv\n");
            printf("6: MPI_Send + MPI_Irecv\n");
            printf("7: MPI_Isend + MPI_Irecv\n");
            printf("8: MPI_Rsend + MPI_Irecv\n");
            printf("9: MPI_Bsend + MPI_Irecv\n");
            printf("10: MPI_Ssend + MPI_Irecv\n");
        }
        MPI_Finalize();
        return 0;
    } else {
        n = strtol(argv[1], (char **) NULL, 10);
        if (argc >= 3) {
            metodo_comunicacao = atoi(argv[2]);
            if (metodo_comunicacao < 1 || metodo_comunicacao > 10) {
                if (meu_ranque == 0) {
                    printf("Método de comunicação deve estar entre 1 e 10\n");
                }
                MPI_Finalize();
                return 0;
            }
        }
    }
    
    // Alocação de buffer para MPI_Bsend
    int buffer_size = MPI_BSEND_OVERHEAD + sizeof(int);
    char *buffer = NULL;
    if (metodo_comunicacao == 4 || metodo_comunicacao == 9) {
        buffer = (char *) malloc(buffer_size);
        MPI_Buffer_attach(buffer, buffer_size);
    }
    
    // Iniciar contagem de tempo
    t_inicial = MPI_Wtime();
    
    // Cada processo calcula primos em uma parte do intervalo
    inicio = 3 + meu_ranque * 2;
    salto = num_procs * 2;
    
    // Procurar números primos na faixa designada
    for (i = inicio; i <= n; i += salto) {
        if (primo(i) == 1) cont++;
    }
    
    // Tratamento especial para métodos 3 e 8 (MPI_Rsend)
    if (metodo_comunicacao == 3 || metodo_comunicacao == 8) {
        // Para garantir sincronização adequada com MPI_Rsend
        
        if (meu_ranque == 0) {
            // Processo 0: inicializa array para armazenar resultados
            int *resultados = (int*)malloc(sizeof(int) * num_procs);
            MPI_Request *requests = NULL;
            
            // Armazena o próprio resultado do processo 0
            resultados[0] = cont;
            
            if (metodo_comunicacao == 8) {
                // Para MPI_Irecv, alocamos requests
                requests = (MPI_Request*)malloc(sizeof(MPI_Request) * (num_procs - 1));
                
                // Postamos todos os recebimentos não-bloqueantes
                for (i = 1; i < num_procs; i++) {
                    MPI_Irecv(&resultados[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD, &requests[i-1]);
                }
            }
            
            // Barreira para sinalizar que os recebimentos foram postados
            MPI_Barrier(MPI_COMM_WORLD);
            
            if (metodo_comunicacao == 3) {
                // Para MPI_Recv, recebemos bloqueantemente após a barreira
                for (i = 1; i < num_procs; i++) {
                    MPI_Recv(&resultados[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
                }
            } else { // metodo_comunicacao == 8
                // Para MPI_Irecv, esperamos que as operações completem
                MPI_Waitall(num_procs - 1, requests, MPI_STATUSES_IGNORE);
                free(requests);
            }
            
            // Soma todos os resultados
            total = 0;
            for (i = 0; i < num_procs; i++) {
                total += resultados[i];
            }
            
            free(resultados);
        } else {
            // Outros processos esperam a barreira e então enviam
            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Rsend(&cont, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    } else {
        // Para todos os outros métodos de comunicação
        if (meu_ranque != 0) {
            // Processos diferentes de 0 enviam seus resultados
            switch (metodo_comunicacao) {
                case 1: // MPI_Send + MPI_Recv
                    MPI_Send(&cont, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                    break;
                    
                case 2: // MPI_Isend + MPI_Recv
                    MPI_Isend(&cont, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
                    MPI_Wait(&request, &status);
                    break;
                    
                case 4: // MPI_Bsend + MPI_Recv
                    MPI_Bsend(&cont, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                    break;
                    
                case 5: // MPI_Ssend + MPI_Recv
                    MPI_Ssend(&cont, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                    break;
                    
                case 6: // MPI_Send + MPI_Irecv
                    MPI_Send(&cont, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                    break;
                    
                case 7: // MPI_Isend + MPI_Irecv
                    MPI_Isend(&cont, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
                    MPI_Wait(&request, &status);
                    break;
                    
                case 9: // MPI_Bsend + MPI_Irecv
                    MPI_Bsend(&cont, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                    break;
                    
                case 10: // MPI_Ssend + MPI_Irecv
                    MPI_Ssend(&cont, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                    break;
            }
        } else {
            // Processo 0 recebe e soma os resultados
            total = cont; // Começa com os primos encontrados pelo processo 0
            
            for (i = 1; i < num_procs; i++) {
                int primos_recebidos = 0;
                
                switch (metodo_comunicacao) {
                    case 1: // MPI_Send + MPI_Recv
                    case 2: // MPI_Isend + MPI_Recv
                    case 4: // MPI_Bsend + MPI_Recv
                    case 5: // MPI_Ssend + MPI_Recv
                        MPI_Recv(&primos_recebidos, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
                        total += primos_recebidos;
                        break;
                        
                    case 6: // MPI_Send + MPI_Irecv
                    case 7: // MPI_Isend + MPI_Irecv
                    case 9: // MPI_Bsend + MPI_Irecv
                    case 10: // MPI_Ssend + MPI_Irecv
                        MPI_Irecv(&primos_recebidos, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &request);
                        MPI_Wait(&request, &status);
                        total += primos_recebidos;
                        break;
                }
            }
        }
    }
    
    // Liberar buffer se foi alocado
    if ((metodo_comunicacao == 4 || metodo_comunicacao == 9) && buffer) {
        MPI_Buffer_detach(&buffer, &buffer_size);
        free(buffer);
    }
    
    // Finalizar contagem de tempo
    t_final = MPI_Wtime();
    
    // Processo 0 imprime o resultado final
    if (meu_ranque == 0) {
        // Adiciona o número 2, que também é primo
        total += 1;
        
        printf("Método de comunicação utilizado: %d\n", metodo_comunicacao);
        printf("Quantidade de primos entre 1 e %ld: %d\n", n, total);
        printf("Tempo de execução: %1.3f segundos\n", t_final - t_inicial);
    }
    
    MPI_Finalize();
    return 0;
}