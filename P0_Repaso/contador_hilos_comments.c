/*
**    Programa para ilustrar el uso de los hilos
**    y memoria compartida.
**      
**    Compilar con : gcc arreglo_hilos.c -lpthread -o arreglo_hilos 
**
*/

/* Biblioteca para crear y usar hilos */
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>


/* Este contador global es COMPARTIDO por todos los hilos (main y nuevo). */
int contador;

/* Esta función es para que el hilo la ejecute. */
void muestraContador(void *ptr) { 
  /* Esto lo ejecutará el hilo nuevo. */

  /* Termina mi código :( */
  /* Debo mostrar continuamente el valor de contador */
  while (1) {
    printf("\nValor actual del contador: %d\n", contador);
    usleep(500000);
  }
}

int main()
{ 
    /**** Comienza solamente el hilo MAIN. ****/
    
    /* El hilo principal inicializa el contador. */
    contador = 0;
        
    /* Crear un hilo para mostrar períodicamente el valor del contador.  */
    pthread_t hilo;
    pthread_create(&hilo, NULL, (void *) muestraContador, NULL);

    /* ESTO YA NO LO EJECUTA EL HILO MAIN. */
 
    /* Incrementar el contador LENTAMENTE. */    
    for (int i=0; i < 5; i++) {
       contador++;
       usleep(1000000);
    }
    
    return 0;
}
