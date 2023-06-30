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
void muestraContador(void *nIterPtr) {
   
  int* intPtr = (int *) nIterPtr; // Convertir a apuntador a entero.
  int nIter = *intPtr;            // Obtener valor de la variable a donde apunta. 
  
  int i = 1;
  
  while(i <= nIter) {
    printf("\nValor actual del contador en la iteración %d: %d\n", i, contador);
    usleep(500000);
    i++;
  }
  
  printf("\nAdios del hilo.\n");
}

int main()
{ 
    /**** Comienza solamente un hilo. ****/
    
    /* El hilo principal inicializa el contador. */
    contador = 0;
        
    /* Crear un hilo para mostrar períodicamente el valor del contador.  */
    pthread_t hilo;
    int numIter = 30;
    pthread_create(&hilo, NULL, (void *) muestraContador, &numIter);


    /* ESTO YA NO LO EJECUTA EL HILO. */
 
    /* Incrementar el contador LENTAMENTE. */    
    for (int i=0; i < 5; i++) 
    {
      contador++;
      usleep(2000000);
    }
    
    printf("\nTermina el proceso.\n\n");
     
    //TODO: pthread_exit: utilizado para terminar un hilo
    //El pthread_exit hace que el hilo main ya no ejecute nada más después.
    //Pero el hilo creado se sigue ejecutandose hasta terminar.
    pthread_exit(NULL);
    printf("ODAAAA\n"); //No se ejecuta porque está después del pthread_exit

    return 0;
}
