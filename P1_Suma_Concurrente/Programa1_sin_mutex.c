/*
 * Programa 1 sin mutex de la práctica 1 de Cómputo Concurrente
 *
 * Profesor: Antonio Lopez Jaimes
 * Autores:
 *  - Brian Rivera Martinez
 *  - Jorge Infante Fragoso
 *
 * Descripción: Escribir un programa multihilado que de manera colaborativa
 * realice la suma de los elementos de un arreglo. La suma final debe estar en
 * una variable compartida.
 *
 * Compilar con: gcc Programa1 -lpthread
 * Ejecutar con: ./a.out <N> <H>
 */

/* Bibliotecas ocupadas */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int sumaTotal = 0; // Variable compartida para la suma total

/* Estructura con argumentos del hilo */
struct argumentos {
  long int *array;
  int *distribucion;
  int iteracion;
  int inicio;
};

/* Funciones a ocupar */
void inicializarArreglo(long int *array, int N);
void mostrarArreglo(long int *array, int N);
void calculoDistribucion(int *distribucion, int N, int H);
int sumaEsperada(int N);
void *sumaParcial(void *ptr);

int main(int argc, char *argv[]) {

  int N = strtol(argv[1], NULL, 10);
  int H = strtol(argv[2], NULL, 10);

  int inicio = 0;

  long int arreglo[N];
  int distribucion[H];

  pthread_t hiloSumador[H];

  memset(arreglo, 0, N * sizeof(arreglo[0]));

  inicializarArreglo(arreglo, N);
  mostrarArreglo(arreglo, N);
  calculoDistribucion(distribucion, N, H);

  struct argumentos argumentosHilo;

  printf("Distribucion: ");
  for (int i = 0; i < H; i++) {
    printf("%i ", distribucion[i]);
  }
  printf("\n\n");

  argumentosHilo.array = arreglo;
  argumentosHilo.distribucion = distribucion;
  argumentosHilo.inicio = inicio;

  for (int i = 0; i < H; i++) {
    argumentosHilo.iteracion = i;
    pthread_create(&hiloSumador[i], NULL, (void *)sumaParcial, &argumentosHilo);
  }

  for (int i = 0; i <= H; i++) {
    pthread_join(hiloSumador[i], NULL);
  }

  printf("Suma esperada: %i\n", sumaEsperada(N));
  printf("Suma obtenida: %i\n", sumaTotal);

  return 0;
}

/*
 * Función que llena el array desde 1 hasta N
 */
void inicializarArreglo(long int *array, int N) {

  long int valor = 1;
  for (int i = 0; i < N; i++) {
    array[i] = valor;
    valor += 1;
  }
}

/*
 * Función que muestra el arreglo generado
 */
void mostrarArreglo(long int *array, int N) {

  printf("\nEl arreglo: ");
  for (int i = 0; i < N; ++i)
    printf("%ld ", array[i]);

  printf("\n\n");
}

/*
 * Función que calcula la distribucion de los hilos, almacenada en un array
 */
void calculoDistribucion(int *distribucion, int N, int H) {

  int aux = 0;

  memset(distribucion, 0, H * sizeof(distribucion[0])); // Llena de 0's el array

  for (int i = 0; i < N; i++) {
    distribucion[aux] += 1;
    aux += 1;

    if (aux == H) {
      aux = 0;
    }
  }
}

/*
 * Función que calcula la suma desde 1 hasta N con formula
 */
int sumaEsperada(int N) {

  int resultado = N * (N + 1) / 2;

  return resultado;
}

/*
 * Función que calcula la suma desde 1 hasta N con hilos
 */
void *sumaParcial(void *ptr) {

  /* Se hace el casteo de la estructura */
  struct argumentos *p = (struct argumentos *)ptr;

  /* Se indica en inicio y se calcula el final de la suma */
  int auxInicio = (p->inicio);
  int final = auxInicio + p->distribucion[p->iteracion];

  /* Se calcula la suma sin candados */
  for (int i = auxInicio; i < final; i++) {
    sumaTotal += p->array[i];
  }

  /* Se calcula el inicio de la suma */
  int auxFinal = final;
  p->inicio = auxFinal;

  return NULL;
}
