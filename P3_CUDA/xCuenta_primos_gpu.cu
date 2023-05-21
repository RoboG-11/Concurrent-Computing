/*
Concurrent Computing Practice Code 3: Introduction to CUDA.

Authors:
        - Rivera Martinez Brian 2193036861.
        - Infante Fragoso Jorge 2203025256.

Professor:
        Lopez Jaimes Antonio.

Problem:
        Dado un arreglo de tamaño N con números impares aleatorios, debemos
        regresar cuántos números primos hay.

Restrictions:
        - El tamaño del arreglo debe ser N=10,000,000 o mayor.
        - Los enteros del arreglo deben ser enteros impares mayores a 1,000,000.
*/

/* Bibliotecas a ocupar */
#include "../common/book.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Prototipo de funciones a ocupar */
void llenarArreglo(int *arr, int tam);
void printArreglo(int *arr, int tam);

/* Tamaño del array */
#define N (1 * 100)

/**
 * @brief Valia si un número es primo
 *
 * Esta función valida si todos el número que se le pasa como
 * parámetro es primo.
 *
 * @param n Es el número a validar si es primo.
 * @return 1 en caso de ser primo, 0 en caso contrario.
 */
__device__ int esPrimo(int n) {
  if (n % 3 == 0)
    return 0;

  for (int i = 5; i * i <= n; i = i + 6) {
    if (n % i == 0 || n % (i + 2) == 0) {
      return 0;
    }
  }
  return 1;
}

/**
 * @brief Kernel de marcar si es primo
 *
 * Este kernel marcar en el array siNo con 1 en caso de que el número en el
 * array dev_enteros sea primo, de lo contrario lo marca con un cero.
 *
 * @param dev_eneteros Es el array que se va validar si es primo.
 * @param dev_siNo El array que se va a mapear con unos o ceros para marcar los
 * números primos.
 * @return void.
 */
__global__ void marcarPrimos(int *dev_enteros, int *dev_siNo) {
  int tid = threadIdx.x + blockIdx.x * blockDim.x;

  while (tid < N) {
    dev_siNo[tid] = esPrimo(dev_enteros[tid]);
    tid += blockDim.x * gridDim.x;
  }
}

/* Función principal del CPU */
int main(int argc, char const *argv[]) {

  /* Arreglo que tendrá los enteros que queremos verificar si son primos o no */
  int *enteros;
  int *siNo;

  /* Se crean los arreglos en la GPU */
  int *dev_enteros, *dev_siNo;

  /* Inicializar la semilla para generar números aleatorios. */
  srand(time(NULL));

  /* Crear los arreglos de enteros y de resultados (sí/no). */
  enteros = (int *)malloc(N * sizeof(int));
  siNo = (int *)malloc(N * sizeof(int));

  /* Se reserva memoria dinámica en la GPU para los arreglos */
  HANDLE_ERROR(cudaMalloc((void **)&dev_enteros, N * sizeof(int)));
  HANDLE_ERROR(cudaMalloc((void **)&dev_siNo, N * sizeof(int)));

  /* Se llena el arreglo 'enteros' con números impares en la CPU */
  llenarArreglo(enteros, N);

  /* Se copia el arreglo 'enteros' y 'siNo' a la GPU */
  HANDLE_ERROR(cudaMemcpy(dev_enteros, enteros, N * sizeof(int),
                          cudaMemcpyHostToDevice));
  HANDLE_ERROR(cudaMemcpy(dev_siNo, siNo, N * sizeof(int), 
                          cudaMemcpyHostToDevice));

  /* Se llama a la función de la GPU */
  marcarPrimos<<<32, 1024>>>(dev_enteros, dev_siNo);

  /* Se copia el array 'siNo' de la GPU a la CPU */
  HANDLE_ERROR(cudaMemcpy(siNo, dev_siNo, N * sizeof(int), 
                          cudaMemcpyDeviceToHost));

  printf("\nArreglo de enteros:\n\n");
  printArreglo(enteros, N);
  printf("\n");
  printf("\nArreglo SiNo\n");
  printArreglo(siNo, N);

  /* Se suman los números que son primos e impares */
  int suma = 0;
  for (int i = 0; i < N; i++)
    suma += siNo[i];

  printf("\nHay %d primos\n\n", suma);

  /* Se libera la memoria dinámica de la CPU */
  free(enteros);
  free(siNo);

  /* Se libera la memoria dinámica de la GPU */
  HANDLE_ERROR(cudaFree(dev_enteros));
  HANDLE_ERROR(cudaFree(dev_siNo));

  return 0;
}

/**
 * @brief Llena el array
 *
 * Esta función llena el array que se le pasa como parámetro,
 * utilizando números impres.
 *
 * @param arr Es el array que se va a llenar.
 * @param tam Es el tamaño del rray.
 * @return void.
 */
void llenarArreglo(int *arr, int tam) {
  for (int i = 0; i < tam; i++) {
    arr[i] = 10000000 + rand() % 10000;
    arr[i] = 2 * arr[i] + 1;
  }
}

/**
 * @brief Imprime el array.
 *
 * Esta funcíon imprime el arreglo que se le pasa como parámetro.
 *
 * @param arr Es el array que se va a imprimir.
 * @param tam Es el tamaño del array.
 * @return void.
 */
void printArreglo(int *arr, int tam) {
  for (int i = 0; i < tam; i++)
    printf("%d ", arr[i]);
}