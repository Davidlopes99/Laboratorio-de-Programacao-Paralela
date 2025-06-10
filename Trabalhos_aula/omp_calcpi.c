#include <stdio.h>
#include <omp.h>
#include <string.h> 

#define N 1000000000UL 

void calculate_pi_with_schedule(const char* schedule_name) {
    double start_time, end_time, pi_value = 0.0;
    long i;

    start_time = omp_get_wtime();

    if (strcmp(schedule_name, "static") == 0) {
        #pragma omp parallel for schedule(static) reduction(+: pi_value)
        for (i = 0; i < N; i++) {
            double t = (double)((i + 0.5) / N);
            pi_value += 4.0 / (1.0 + t * t);
        }
    } else if (strcmp(schedule_name, "static3") == 0) {
        #pragma omp parallel for schedule(static, 3) reduction(+: pi_value)
        for (i = 0; i < N; i++) {
            double t = (double)((i + 0.5) / N);
            pi_value += 4.0 / (1.0 + t * t);
        }
    } else if (strcmp(schedule_name, "dynamic") == 0) {
        #pragma omp parallel for schedule(dynamic) reduction(+: pi_value)
        for (i = 0; i < N; i++) {
            double t = (double)((i + 0.5) / N);
            pi_value += 4.0 / (1.0 + t * t);
        }
    } else if (strcmp(schedule_name, "dynamic3") == 0) {
        #pragma omp parallel for schedule(dynamic, 3) reduction(+: pi_value)
        for (i = 0; i < N; i++) {
            double t = (double)((i + 0.5) / N);
            pi_value += 4.0 / (1.0 + t * t);
        }
    } else if (strcmp(schedule_name, "guided") == 0) {
        #pragma omp parallel for schedule(guided) reduction(+: pi_value)
        for (i = 0; i < N; i++) {
            double t = (double)((i + 0.5) / N);
            pi_value += 4.0 / (1.0 + t * t);
        }
    } else if (strcmp(schedule_name, "guided3") == 0) {
        #pragma omp parallel for schedule(guided, 3) reduction(+: pi_value)
        for (i = 0; i < N; i++) {
            double t = (double)((i + 0.5) / N);
            pi_value += 4.0 / (1.0 + t * t);
        }
    } else { 
        #pragma omp parallel for reduction(+: pi_value)
        for (i = 0; i < N; i++) {
            double t = (double)((i + 0.5) / N);
            pi_value += 4.0 / (1.0 + t * t);
        }
    }

    end_time = omp_get_wtime();

    printf("Schedule: %s\n", schedule_name);
    printf("Valor de pi: %.12f\n", pi_value / N);
    printf("Tempo de execução: %.6f segundos\n\n", end_time - start_time);
}

int main() {
    printf("Começando Benchmark...\n\n");

    calculate_pi_with_schedule("base");
    calculate_pi_with_schedule("static");
    calculate_pi_with_schedule("static3");
    calculate_pi_with_schedule("dynamic");
    calculate_pi_with_schedule("dynamic3");
    calculate_pi_with_schedule("guided");
    calculate_pi_with_schedule("guided3");

    printf("Benchmarks Finished.\n");

    return 0;
}