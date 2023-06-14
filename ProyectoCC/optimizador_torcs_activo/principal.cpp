// Compilar con:

/* INSTALAR LA BIBLIOTECA fmt PARA PODER COMPILAR SIN PROBLEMAS. */

// VERSION SECUENCIAL:
// $> g++ -I./problemas/ ./problemas/*.cpp *.cpp -lfmt -o genetico

// VERSION CONCURRENTE CON MPI:
// $> mpic++ -I./problemas/ ./problemas/*.cpp *.cpp -lfmt -o genetico

#include <iostream>
#include "GeneticoSimple.h"
#include "TorcsFun.h"
#include "CannonFun.h"

using namespace std;


int main(int argc, char *argv[])
{
  /* Se inicializa el entorno MPI */
  MPI_Init(&argc, &argv);

  /* Se obtienen el número de procesos */
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  /* Se obtiene el rank del proceso */
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  int myRank=world_rank;
  /*
   * Para la versión secuencial se usa serverID=0, PERO
   * para la versión concurrente serverID debe ser igual a myRank.
   */
  int serverID = world_rank;

  /* Aquí se crea la instancia de TorcsFun con el serverID indicado. */
  ProblemaOptim *problema = new TorcsFun(serverID);

  /* Para usar el problema del Controlador del Auto en TORCS
   * deben descomentar la línea de arriba y comentar la de abajo.
   */

  /* Con esta instancia se resolverá el problema del cañón. */
  // ProblemaOptim* problema = new CannonFun();

  /* La descripción de estos valores está en GeneticoSimple.h */
  ParamsGA pars = {4,   /* popSize  */
                  2,   /* Gmax */
                  0.9, /* Pc */
                  0.1, /* Pm */
                  6,   /* precision */
                  10,  /* nMigrantes*/
                  16,  /* tamEpoca*/};

  GeneticoSimple ga(problema, pars);
  ga.optimizar();

  delete problema;
    /* Se finaliza el entorno MPI */
  MPI_Finalize();
  return 0;
}
