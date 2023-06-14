/*
 * GeneticoSimple.cpp
 * Implemetación de un algoritmo genético simple con representación binaria,
 * selección proporcional de ruleta, cruza en un punto, y mutación uniforme.
 */

#include <ctime>
#include <cmath>
#include <limits>
#include "GeneticoSimple.h"
#include <fmt/core.h>
#include <fmt/ranges.h>

GeneticoSimple::GeneticoSimple(ProblemaOptim *p, ParamsGA &params)
{
   problema = p;
   popSize = params.popSize;
   Gmax = params.Gmax;
   Pc = params.Pc;
   Pm = params.Pm;
   precision = params.precision;

   random_device rd;
   rng.seed(rd());

   oldpop = new Individuo[popSize];
   newpop = new Individuo[popSize];
   //REVISAR 
   globalpop = new Individuo[popSize];

   padres.assign(popSize, 0);
   stats.reset(problema, precision);

   tiempoMaximo = 0.0; // el PEOR tiempo en terminar la carrera.

   // Crear directorio de salida, si aún no existe.
   /*** En la versión CONCURRENTE/MPI solamente la isla RAIZ debería crear el directorio de salida. ***/
   if (myRank == RAIZ){
      if (!stats.dirExists("./salidafinal/")){
         const int dir_err = mkdir("./salidafinal/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
         if (dir_err == -1){
            cerr << "\nNo se pudo crear el directorio de salida '/salidafinal/"<< "\n\n";
            exit(1);
         }
      }
   }
   elegidos.resize(nMigrantes);
   /*** En la versión CONCURRENTE/MPI, todas las islas deben esperar la creación del directorio de salida con Barrier. ***/
   MPI_Barrier(MPI_COMM_WORLD);
}

GeneticoSimple::~GeneticoSimple()
{
   delete[] newpop;
   delete[] oldpop;
   if (myRank == RAIZ)
      delete[] globalpop;
}

void GeneticoSimple::optimizar()
{
   Individuo *temp;

   /* Inicializar la población y reportar las estadísticas iniciales */
   gen = 1; /* La generación aleatoria es la primera. */
   stats.initial_report(cout, popSize, Gmax, Pc, Pm);

   inicalizarPob();

   evaluarPoblacion(oldpop);
   elitismo(oldpop, gen);
   stats.statistics(oldpop, popSize);

   for (gen = 2; gen <= Gmax; gen++)
   {
      /* Seleccionar los padres guiados por la aptitud. */
      seleccionPadres(oldpop);

      /* Cruzar los pares de padres para producir la población de hijos. */
      cruza(oldpop, newpop);

      /* Mutar a los hijos según Pm. */
      mutacion(newpop);

      /* Evaluar la nueva generación */
      evaluarPoblacion(newpop);
      elitismo(newpop, gen); // Encontrar el mejor individuo.

      if(gen%tamEpoca==0){

      }


      /* Calcular las estadísticas sobre la aptitud en la nueva generación */
      stats.statistics(newpop, popSize);

      /* Imprimir los resultados de las estadísticas */
      // stats.report(cout, oldpop, newpop, popSize, gen);  // Esto imprime TODA la información.
      stats.shortReport(cout, oldpop, newpop, popSize, gen);

      /* Ahora, la nueva generación será la vieja */
      temp = oldpop;
      oldpop = newpop;
      newpop = temp;
   }

   // Para dejar las variables (PESOS) de la población final en este archivo.
   // ***salidafinal*** debe estar donde corren este programa.
   ofstream archVariables("./salidafinal/pesos_pob.txt", std::ofstream::out);

   // Para dejar la evaluación (tiempo y distancia restante) de la población.
   ofstream archEvaluacion("./salidafinal/evals_pob.txt", std::ofstream::out);

   stats.writeVariables(archVariables, oldpop, popSize);
   stats.writeEvaluation(archEvaluacion, oldpop, popSize);
   archVariables.close();
   archEvaluacion.close();
}

/* Evaluación de cada uno de los popsize individuos */
void GeneticoSimple::evaluarPoblacion(Individuo *pop)
{

   for (int i = 0; i < popSize; ++i)
   {
      problema->evaluateFun(pop[i].x, pop[i].eval, pop[i].cons);

      stats.nevals++;

      /*** Para considerar una restricción en el problema que se resuelve
       * eval: tiempo de carrera.
       * cons[0]: distancia para terminar la carrera.
       */

      /*** tiempoMaximo debe agregarse a la clase e inciarse en CERO en el contructor ***/

      // Actualizar el tiempo máximo
      if (tiempoMaximo < pop[i].eval)
         tiempoMaximo = pop[i].eval;

      if (pop[i].eval == 0.0) // el tiempo es 0, NO terminó el auto.
         pop[i].aptitud = 1 / (pop[i].cons[0] + tiempoMaximo);
      else // el auto sí terminó.
         pop[i].aptitud = 1 / pop[i].eval;
   }
}

/*
 *  En la versión CONCURRENTE/MPI, deben iniciar el arreglo globalpop de
 *  manera parecida a como se inicia oldpop o newpop.
 *
 *  El método insuflar reserva espacio para el vector x, cons, y el cromosoma.
 *
 */
   //REVISAR globalpop = new Individuo[popSize];

/* Creación aleatoria de la población inicial */
void GeneticoSimple::inicalizarPob()
{
   for (int j = 0; j < popSize; j++)
   {
      oldpop[j].insuflar(problema, precision);
      newpop[j].insuflar(problema, precision);
   }

   Pm = 1.0 / oldpop[0].chromoSize;
}

/* Acá deben podrían poner sus métodos nuevos para la versión CONCURRENTE/MPI */
void GeneticoSimple::migracion(Individuo* pop){
   cout << "Este es el metodo de migración";
   std::vector<int> elegidos(nMigrantes);
   obtenElegidos(elegidos,nMigrantes);
   bufSize=nMigrantes*(problema->numVariables()+1)*sizeof(double)
   char* buffer = new char[bufSize];
   int position=0;
   for (i=0; i < N_MIG; i++) {
      MPI_Pack(pop[elegidos[i]].x.data(), problema->numVariables, MPI_DOUBLE, buffer, bufSize, 
            &position, MPI_COMM_WORLD);
      MPI_Pack(&(pop[elegidos[i]].aptitud), 1, MPI_DOUBLE, buffer, bufSize, 
            &position, MPI_COMM_WORLD);
   }

   printf("\nPosicion: %d\n", position);

   /*
   if(world_rank == 0){
      MPI_Send(void* buffer, position, MPI_PACKED, myRank+1, 0, MPI_COMM_WORLD);
      MPI_Recv(void* buffer, position, MPI_PACKED, world_size-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
      printf("HOST recibe: %d\n",x);
   }else{
      MPI_Recv(&x,1,MPI_INT,world_rank-1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE); 
      x=x+3;
      MPI_Send(&x,1,MPI_INT,(world_rank+1)%world_size,0,MPI_COMM_WORLD); 
      printf("TRABAJADOR no. %d envía: %d\n",world_rank,x);
   }*/
}

void GeneticoSimple::fillArray(int *array){
   int i;
   for(i=0;i<popSize;i++){
      array[i]=i;
   }
}

void GeneticoSimple::obtenElegidos(vector<int>& elegidos, int nMigrantes){
   int array[popSize];
   int decrementarPopSize = popSize;
   fillArray(array);
   int i;
   int numAleatorio;
   for(i=0;i<nMigrantes;i++){
      numAleatorio=rand%decrementarPopSize;
      elegidos.push_back(array[numAleatorio]);
      array[numAleatorio]=array[decrementarPopSize-1];
      decrementarPopSize--;
   }
}

void GeneticoSimple::unionPoblaciones(Individuo* pop){

}


/*
 **************************************************************
 * LOS MÉTODOS QUE ESTÁN ABAJO YO NO NECESITAN MODIFICARLOS
 *
 *
 **/

/* Sección de los padres según su aptitud. */
void GeneticoSimple::seleccionPadres(Individuo *pop)
{
   calcularValEsperado(pop);

   for (int i = 0; i < popSize; i++)
      padres[i] = seleccionRuleta(pop);
}

/* El algoritmo de selección proporcional de Ruleta */
int GeneticoSimple::seleccionRuleta(Individuo *pop)
{
   double suma, ale;
   int j;
   uniform_real_distribution<> rdis(0.0, sumvalesp);

   ale = rdis(rng);
   suma = 0.0;
   j = 0;

   do
   {
      suma += pop[j].valesp;
      j++;
   } while (suma < ale && j < popSize);

   return j - 1;
}

/* Crear la nueva generación por medio de selección, cruza y mutación */
void GeneticoSimple::cruza(Individuo *oldpop, Individuo *newpop)
{
   int mate1, mate2, pcruza;

   // En cada iteración cruzar dos padres y mutar los 2 hijos
   for (int j = 0; j < popSize - 1; j += 2) // <--- j avanza de 2 en 2.
   {
      mate1 = padres[j];     // índice del padre 1
      mate2 = padres[j + 1]; // índice del padre 2

      /* Se efectúa la cruza para producir dos nuevos individuos */
      pcruza = cruza1Punto(oldpop[mate1].cromo, oldpop[mate2].cromo,
                           newpop[j].cromo, newpop[j + 1].cromo);
      newpop[j].xsite1 = pcruza;
      newpop[j + 1].xsite1 = pcruza;
      newpop[j].padres[0] = mate1 + 1;
      newpop[j].padres[1] = mate2 + 1;
      newpop[j + 1].padres[0] = mate1 + 1;
      newpop[j + 1].padres[1] = mate2 + 1;
   }
}

/* El operador de cruza aplicado a un individuo dado */
int GeneticoSimple::cruza1Punto(Cromosoma &padre1, Cromosoma &padre2,
                                Cromosoma &hijo1, Cromosoma &hijo2)
{
   int pcruza, j;
   int chromoSize = padre1.size();

   /* Para elegir el punto de cruza entre 0 y chromoSize-1 */
   uniform_int_distribution<int> unif(0, chromoSize - 1);

   /* Realizar la cruza con una probabilidad Pc */
   if (flip(Pc))
   {
      pcruza = unif(rng);
      /* Realizar la cruza */
      for (j = (chromoSize - 1); j >= (chromoSize - pcruza); j--)
      {
         hijo1[j] = padre1[j];
         hijo2[j] = padre2[j];
      }

      for (j = (chromoSize - pcruza) - 1; j >= 0; j--)
      {
         hijo1[j] = padre2[j];
         hijo2[j] = padre1[j];
      }

      stats.ncruzas++; /* Guardar el número de cruzas */
   }
   else
   { /* los padres pasan idénticos a la siguiente generación */
      hijo1 = padre1;
      hijo2 = padre2;
      pcruza = 0;
   }

   return pcruza;
}

void GeneticoSimple::mutacion(Individuo *pop)
{
   // En cada iteración cruzar dos padres y mutar los 2 hijos
   for (int j = 0; j < popSize; j++)
   {
      /* Mutar (si es el caso) los hijos resultantes de la cruza. */
      pop[j].nMutaciones = mutacionUniforme(pop[j].cromo);
      pop[j].decodificar();

      if (pop[j].nMutaciones > 0)
         stats.nmutaciones++; /* Solamente registra cuántos individuos mutaron. */
   }

   /* Mantener al mejor individuo colocándolo en una posición aleatoria */
   uniform_int_distribution<int> unif(0, popSize - 1);
   int ale = unif(rng);
   pop[ale].copiar(&stats.bestfit);
   stats.positionBestFit = ale + 1;
}

/*  El operador de mutación aplicado a un individuo dado */
int GeneticoSimple::mutacionUniforme(Cromosoma &cromo)
{
   int numMutations = 0;
   /* Recorrer todos los bits y mutar si es necesario. */
   for (unsigned k = 0; k < cromo.size(); k++)
   {
      if (flip(Pm))
      {
         numMutations++;
         cromo[k] = (cromo[k] == 0) ? 1 : 0;
      }
   }

   return numMutations;
}

void GeneticoSimple::calcularValEsperado(Individuo *pop)
{
   /* Sumar la aptitud */
   double sumaptitud = 0.0;
   for (int j = 0; j < popSize; j++)
      sumaptitud = sumaptitud + pop[j].aptitud;

   /* Calcular el promedio de la aptitud */
   stats.avgApt = sumaptitud / popSize;

   /* Calcular la suma del valor esperado de los individuos */
   sumvalesp = 0.0; /* Este valor se usa en la selección por ruleta. */
   for (int j = 0; j < popSize; j++)
   {
      if (stats.avgApt != 0.0)
         pop[j].valesp = pop[j].aptitud / stats.avgApt;
      else
         pop[j].valesp = 0.0f;

      sumvalesp += pop[j].valesp;
   }
}

void GeneticoSimple::elitismo(Individuo *pop, int gen)
{
   for (int j = 0; j < popSize; j++)
   {
      /* Probar si se ha encontrado un nuevo máximo GLOBAL */
      if (pop[j].aptitud > stats.bestfit.aptitud)
      {
         stats.bestfit.copiar(&pop[j]);
         stats.generationBestFit = gen;
         stats.positionBestFit = j + 1;
      }
   }
}

/* Lanzamiento de una moneda sesgada - true si cae cara */
int GeneticoSimple::flip(double prob)
{
   uniform_real_distribution<> rdis(0.0, 1.0);

   if (rdis(rng) <= prob)
      return true;
   else
      return false;
}

string GeneticoSimple::nombreArchivoPesos(int i) const
{
   char buffer[200];
   sprintf(buffer, patronArchivoVars.c_str(), i);
   return string(buffer);
}

string GeneticoSimple::nombreArchivoEvals(int i) const
{
   char buffer[200];
   sprintf(buffer, patronArchivoEval.c_str(), i);
   return string(buffer);
}
