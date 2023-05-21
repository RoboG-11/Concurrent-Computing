/*
**    Programa para ilustrar el uso de los hilos
**    y la memoria compartida.
**
**    Compilar con : gcc cuenta_primos_cpu.c -lpthread -o primos_cpu
**
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

/* Prototipo de la función para que un hilo sume */
void contarPrimos(void *ptr);
bool esPrimo(long n);
void llenarArreglo(int* arr, int tam);
void printArreglo(int* arr, int tam);

#define N (4 * 1024)

/* Arreglo que tendrá los enteros que queremos verificar si son primos o no */
int* enteros;
int* siNo;

/* Esta es una variable compartida por todos los hilos */
long long numPrimos = 0;

int numHilos;

int main(int argc, char const *argv[])
{
    /* Inicializar la semilla para generar números aleatorios. */
    srand(time(NULL));

   /* Verificar que venga como argumento el número de hilos. */
   if (argc < 2) { // Si pasaron menos de 2 argumentos  
      printf("\nDebes correr este programa así:\n");
      printf("\n > %s NUM_HILOS\n", argv[0]);
      
      return 0;
   }
   
   sscanf(argv[1], "%d", &numHilos);
   
   /* Crear los arreglos de enteros y de resultados (sí/no). */
   enteros = (int *) malloc(N * sizeof(int));
   siNo = (int*) malloc(N * sizeof(int));

   llenarArreglo(enteros, N);

   /* Deben declarar un arreglo para manejar los hilos de tipo  pthread_t */
   pthread_t threads[numHilos];
         
   /* Crear los numHilos hilos */
   int id[numHilos];

   printf("\nHay %d hilos.\n", numHilos);
   for (int i = 0; i < numHilos; i++) {
      id[i] = i;
      pthread_create(&threads[i], NULL, (void *) contarPrimos, &id[i]);
   }

   /* El hilo principal debe mostrar el conteo final */
   printf("\nCalculando resultados...\n\n");
   
   for (int i = 0; i < numHilos; i++)
      pthread_join(threads[i], NULL);

   printf("\nArreglo enteros\n");
   printArreglo(enteros, N);
   printf("\nYa terminaron los hilos de identificar números primos.\n\n", numPrimos);
   printf("\nArreglo SiNo\n");
   printArreglo(siNo, N);

    int suma = 0;
    for (int i=0; i < N; i++)
        suma += siNo[i];

    printf("\nHay %d primos\n\n", suma);

   free(enteros);
   free(siNo);
   
   return 0;
}

void contarPrimos(void *ptrID)
{
   /* Tomar identificador del hilo actual */
   int tid = *( (int *) ptrID );
   
   while (tid < N)
   {
      if (esPrimo( enteros[tid] ) == true)
         siNo[tid] = true;
      else
         siNo[tid] = false;
      
      tid += numHilos; // numHilos es global en este programa.
   }
   
}

bool esPrimo(long n) 
{
   bool primo = true;
   
   long p = 2;
   while (primo == true && p < n-1) {
      if (n % p == 0)
         primo = false;
      
      p++;
   }
   
   return primo;
}

void llenarArreglo(int* arr, int tam) {
    for (int i=0; i < tam; i++) {
        arr[i] = 10000000 + rand()%10000;
        arr[i] = 2*arr[i] + 1;  // Para que el entero sea IMPAR.
    }
}

void printArreglo(int* arr, int tam) {
    for (int i=0; i < tam; i++)
        printf("%d ", arr[i]);
}
