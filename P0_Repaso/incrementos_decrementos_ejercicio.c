/*
**    Programa para ilustrar el uso de los hilos
**    y la memoria compartida.
**
**    Compilar con : gcc suma.c -lpthread -o suma
**
*/

#include <pthread.h>
#include <stdio.h>

#define NUM_OPERACIONES 10000

// Contador compartido por todos los hilos
long balance = 0;

/* Prototipos de las funciones que ejecutar�n los hilos */
void sumar(void *ptr);
void restar(void *ptr);

int main() {

  pthread_t sumador;
  pthread_create(&sumador, NULL, (void *)sumar, NULL);

  /* Declarar y crear un hilo "restador" */
  pthread_t restador;
  pthread_create(&restador, NULL, (void *)restar, NULL);

  // Se esperan a los hilos
  pthread_join(sumador, NULL);
  pthread_join(restador, NULL);

  printf("El balance final es: %ld\n", balance);

  pthread_exit(NULL);

  return 0;
}

void sumar(void *ptr) {

  for (int i = 0; i < NUM_OPERACIONES; i++){
    balance = balance + 1;
  }
}

/* ESCRIBIR LA FUNCIÓN restar PARA EL HILO QUE RESTARÁ */
void restar(void *ptr) {
  
  for (int i = 0; i < NUM_OPERACIONES; i++){
    balance = balance - 1;
  }
}
