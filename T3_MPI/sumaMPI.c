/*
Concurrent Computing Practice Code 3: Task 2: Process communication in MPI.

Authors:
  - Rivera Martinez Brian 2193036861.
  - Infante Fragoso Jorge 2203025256.
  - Mario Baltazar Reséndiz 2183034471.

Professor:
  Lopez Jaimes Antonio.

Problem:
  Implementar un algoritmo paralelo utilizando la biblioteca MPI. 
  El objetivo del programa es realizar una operación de suma en paralelo 
  entre los diferentes procesos involucrados. Cada proceso tiene asignado 
  un ID de proceso (world_rank) y se comunica con otros procesos utilizando 
  las funciones MPI_Send y MPI_Recv para intercambiar valores.

Restrictions:
  - El código debe de ser implementado con MPI.
  - El código debe de ser elaborado por máximo 4 personas.

Compilación:
    mpicc Tarea2.c -lm

Ejecución:
    mpiexec -n 8 --hostfile /home/alumnos/a2203025256/lista_nodos.txt ./a.out
*/

/* Bibliotecas a ocupar */
#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

/* Función principal */
int main(int argc, char** argv) {

  /* Se inicializa el entorno MPI */
  MPI_Init(&argc, &argv);

  /* Se obtienen el número de procesos */
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  /* Se obtiene el rank del proceso */
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  /* Variables a ocupar */
  int x = 1;
  int id = world_rank;
  int noRondas = log2(world_size);
  int i = 0;
  int salto;

  /* Bucle para los proceso pares que realizan la suma en paralelo */
  while (id%2 == 0 && i < noRondas){

    /* Se calcula el salto entre procesos */
    salto = pow(2, i);
    
    /* Se recibe el valor del proceso vecino */
    MPI_Recv(&x, 1, MPI_INT,world_rank + salto, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
    
    /* Se duplica el valor recibido */
    x = x + x;

    /* Se actualiza el ID y el contador */
    id = id / 2;
    i++;
  }

  /* Condición para los procesos que no son el principal y en envíán */
  if (world_rank != 0){

    /* Se calcula el salto entre procesos */
    salto = pow(2,i);

    /* Se envía el valor al proceso vecino */
    MPI_Send(&x, 1, MPI_INT, world_rank - salto, 0, MPI_COMM_WORLD);

    printf("ITERACION %d: Soy el trabajador %d y envié: %d\n",i + 1, world_rank, x);

    /* Se actualiza el contador */
    i++;
  }

  /* Condición para el proceso principal */
  if (world_rank == 0){
    printf("Soy el host y recibí: %d\n", x);
  }

  /* Se finaliza el entorno MPI */
  MPI_Finalize();
}