/*
**    Programa para ilustrar el uso de los hilos
**    y memoria compartida.
**
**    Compilar con : gcc arreglo_hilos.c -lpthread -o arreglo_hilos
**
*/

#include <pthread.h>
#include <stdio.h>

#define TAM_ARR 10 /* Tamaño PAR del arreglo compartido. */

/* Este arreglo global es COMPARTIDO por todos los hilos. */
int arreglo[TAM_ARR];

/* Esta función es para que los hilos la ejecuten. */
void calculaCuadrados(void *ptrEntero) {
  int i = 0;

  /* Calcular cuadrados de la parte que me toca. */
  int *intEntero = (int *)ptrEntero; // Se castea para poderlo utilizar
  int number = *intEntero;

  for(i = 0; i < TAM_ARR; i++){
    arreglo[i] = arreglo[i] * arreglo[i];
  }
}

/* Estos dos procedimientos solamente los usa el hilo principal (main). */
void iniciarArreglo(int *arr, int inicio, int fin) {
  for (int i = inicio; i <= fin; ++i)
    arr[i] = i;
}

void mostrarArreglo(int *arr, int inicio, int fin) {
  printf("\nEl arreglo: ");
  for (int i = inicio; i <= fin; ++i)
    printf("%d ", arr[i]);

  printf("\n\n");
}

int main() {
  /**** Comienza solamente un hilo. ****/

  int id = 0;

  /* El hilo principal Inicializa el arreglo. */
  iniciarArreglo(arreglo, 0, TAM_ARR - 1);
  mostrarArreglo(arreglo, 0, TAM_ARR - 1);

  /* Crear el PRIMER hilo para procesar la primera mitad del arreglo.  */
  /* crear_hilo(...); */
  pthread_t hilo1;
  pthread_create(&hilo1, NULL, (void *)calculaCuadrados, &id);
  pthread_join(hilo1, NULL); // Se agregó para que se lo espere y se muestren ambos

  /* Crear el SEGUNDO hilo para procesar la segunda mitad del arreglo. */
  /* crear_hilo(...); */

  /* ESTO YA NO LO EJECUTAN LOS HILOS. */

  /* El hilo principal muestra el arreglo final */
  mostrarArreglo(arreglo, 0, TAM_ARR - 1);

  return 0;
}
