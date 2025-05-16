#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

#define CHUNK 500000

int primo(int n) {
    if (n < 2) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    for (int i = 3; i <= (int)sqrt(n); i += 2)
        if (n % i == 0) return 0;
    return 1;
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 3 || size < 2) {
        if (rank == 0)
            fprintf(stderr, "Uso: %s <N> <metodo 1-9> (P>=2)\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    int N = atoi(argv[1]);
    int metodo = atoi(argv[2]); // 1..9

    // 1..5 = Send, Isend, Rsend, Bsend, Ssend
    // 6..9 = mesmos 1..4, mas com Irecv ao invés de Recv
    int sendCode = ((metodo - 1) % 5) + 1;
    int useIrecv  = (metodo > 5);

    double t0 = MPI_Wtime();
    MPI_Status stat;
    MPI_Request req;

    if (sendCode == 4) {
        // Para Bsend (4) precisamos de buffer
        int bufsize = MPI_BSEND_OVERHEAD + sizeof(int);
        char *buf = malloc(bufsize);
        MPI_Buffer_attach(buf, bufsize);
    }
    
    // Buffer extra para Rsend (método 3)
    int rsend_prep_tag = 99; // Tag especial para preparação do Rsend

    if (rank == 0) {
        int next = 3;
        int active = size - 1;
        int tag;
        
        // Para Rsend: receba sinais de "estou pronto" de cada worker
        if (sendCode == 3) {
            for (int dst = 1; dst < size; dst++) {
                int dummy;
                MPI_Recv(&dummy, 1, MPI_INT, dst, rsend_prep_tag, MPI_COMM_WORLD, &stat);
            }
        }

        // envia o primeiro chunk a cada escravo
        for (int dst = 1; dst < size; ++dst) {
            tag = 1;
            switch (sendCode) {
                case 1: MPI_Send(&next,1,MPI_INT,dst,tag,MPI_COMM_WORLD); break;
                case 2: MPI_Isend(&next,1,MPI_INT,dst,tag,MPI_COMM_WORLD,&req);
                        MPI_Wait(&req,&stat);
                        break;
                case 3: // Rsend
                        MPI_Rsend(&next,1,MPI_INT,dst,tag,MPI_COMM_WORLD);
                        break;
                case 4: // Bsend
                        MPI_Bsend(&next,1,MPI_INT,dst,tag,MPI_COMM_WORLD);
                        break;
                case 5: // Ssend
                        MPI_Ssend(&next,1,MPI_INT,dst,tag,MPI_COMM_WORLD);
                        break;
            }
            next += CHUNK;
        }

        int total = 1;  // contabiliza o 2
        // loop de coleta e redistribuição
        while (active > 0) {
            int cnt;
            int src;
            // recebe resultado
            if (!useIrecv) {
                MPI_Recv(&cnt,1,MPI_INT,MPI_ANY_SOURCE,0,
                         MPI_COMM_WORLD,&stat);
            } else {
                MPI_Irecv(&cnt,1,MPI_INT,MPI_ANY_SOURCE,0,
                          MPI_COMM_WORLD,&req);
                MPI_Wait(&req,&stat);
            }
            src = stat.MPI_SOURCE;
            total += cnt;

            // decide tag de finalização ou novo chunk
            if (next > N) {
                tag = 0;
                --active;
            } else {
                tag = 1;
            }
            
            // Para Rsend: receba sinal de "estou pronto" do worker específico
            if (sendCode == 3) {
                int dummy;
                MPI_Recv(&dummy, 1, MPI_INT, src, rsend_prep_tag, MPI_COMM_WORLD, &stat);
            }
            
            // envia próxima tarefa
            switch (sendCode) {
                case 1: MPI_Send(&next,1,MPI_INT,src,tag,MPI_COMM_WORLD); break;
                case 2: MPI_Isend(&next,1,MPI_INT,src,tag,MPI_COMM_WORLD,&req);
                        MPI_Wait(&req,&stat);
                        break;
                case 3:
                        MPI_Rsend(&next,1,MPI_INT,src,tag,MPI_COMM_WORLD);
                        break;
                case 4:
                        MPI_Bsend(&next,1,MPI_INT,src,tag,MPI_COMM_WORLD);
                        break;
                case 5:
                        MPI_Ssend(&next,1,MPI_INT,src,tag,MPI_COMM_WORLD);
                        break;
            }
            next += CHUNK;
        }

        double t1 = MPI_Wtime();
        printf("[Bag-of-Tasks switch] metodo=%d → primos<=%d: %d  tempo=%1.3fs\n",
               metodo, N, total, t1 - t0);
    }
    else {
        // escravos
        while (1) {
            int start, tag_recv;
            
            // Para Rsend: primeiro notifica o mestre que o receive está postado
            if (sendCode == 3) {
                int ready = 1;
                MPI_Send(&ready, 1, MPI_INT, 0, rsend_prep_tag, MPI_COMM_WORLD);
            }
            
            // recebe tarefa
            if (!useIrecv) {
                MPI_Recv(&start,1,MPI_INT,0,MPI_ANY_TAG,
                         MPI_COMM_WORLD,&stat);
                tag_recv = stat.MPI_TAG;
            } else {
                MPI_Irecv(&start,1,MPI_INT,0,MPI_ANY_TAG,
                          MPI_COMM_WORLD,&req);
                MPI_Wait(&req,&stat);
                tag_recv = stat.MPI_TAG;
            }
            if (tag_recv == 0) break;

            // calcula primos no pedaço
            int cnt = 0;
            for (int i = start; i < start + CHUNK && i <= N; i += 2)
                if (primo(i)) cnt++;
            
            // envia resultado
            switch (sendCode) {
                case 1: MPI_Send(&cnt,1,MPI_INT,0,0,MPI_COMM_WORLD); break;
                case 2: MPI_Isend(&cnt,1,MPI_INT,0,0,MPI_COMM_WORLD,&req);
                        MPI_Wait(&req,&stat);
                        break;
                case 3:
                        MPI_Send(&cnt,1,MPI_INT,0,0,MPI_COMM_WORLD);
                        // Para o próximo Rsend, primeiro notifica o mestre
                        if (tag_recv != 0) { // Não notificar se foi a última iteração
                            int ready = 1;
                            MPI_Send(&ready, 1, MPI_INT, 0, rsend_prep_tag, MPI_COMM_WORLD);
                        }
                        break;
                case 4:
                        MPI_Bsend(&cnt,1,MPI_INT,0,0,MPI_COMM_WORLD);
                        break;
                case 5:
                        MPI_Ssend(&cnt,1,MPI_INT,0,0,MPI_COMM_WORLD);
                        break;
            }
        }
    }

    // barrier final para garantir limpeza
    MPI_Barrier(MPI_COMM_WORLD);
    
    if (sendCode == 4) {
        void *buf;
        int bufsize;
        MPI_Buffer_detach(&buf, &bufsize);
        free(buf);
    }
    
    MPI_Finalize();
    return 0;
}