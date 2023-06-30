/*
**    Programa para ilustrar el uso de los hilos
**    y memoria compartida.
**      
**    Compilar con : gcc arreglo_hilos.c -lpthread -o arreglo_hilos 
**
*/

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

/* Este contador global es COMPARTIDO por todos los hilos (main y nuevo). */
int contador;

/* Esta función es para que el hilo la ejecute. */
/* Le pasaremos un argumento a la función del hilo. */
void secuencia(void *idPtr) {
   
  int* intPtr = (int *) idPtr; // Convertir a apuntador a entero.
  int id = *intPtr;            // Obtener valor de la variable a donde apunta. 
  
  // Esta variable es local, cada hilo tiene la suya.
  int numero=0;  
  
  for (int i = 0; i < 15; i++) {
      numero = numero + id;
      printf("\nHilo %d: el siguiente valor es %d.\n", id, numero);
      usleep(500000);
  }
  
  printf("\nAdios hilo %d\n", id);
}

int main()
{ 
    /**** Comienza solamente un hilo. ****/
    
    /* El hilo principal inicializa el contador. */
    contador = 0;
        
    /* Crear un hilo para mostrar períodicamente el valor del contador.  */
    pthread_t hilo1;
    int idHilo1 = 1;
    pthread_create(&hilo1, NULL, (void *) secuencia, &idHilo1);
    
    pthread_t hilo2;
    int idHilo2 = 2;
    pthread_create(&hilo2, NULL, (void *) secuencia, &idHilo2);
   

    printf("\nAdios hilo MAIN\n");
    pthread_exit(NULL);
    
    return 0;
}
