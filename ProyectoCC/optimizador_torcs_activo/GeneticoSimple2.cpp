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
#include <mpi.h>

using namespace std;

GeneticoSimple::GeneticoSimple(ProblemaOptim *p, ParamsGA &params)
{
   problema = p;
   popSize = params.popSize;
   Gmax = params.Gmax;
   Pc = params.Pc;
   Pm = params.Pm;
   precision = params.precision;
   tamEpoca = params.tamEpoca;
   nMigrantes = params.nMigrantes;
   MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
   MPI_Comm_size(MPI_COMM_WORLD, &numIslas);


   random_device rd;
   rng.seed(rd());
   
   oldpop = new Individuo[popSize];
   newpop = new Individuo[popSize];

   if(myRank == RAIZ){
      globalpop = new Individuo[popSize * numIslas];
   }
   

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
   cout << "\n\n\n\n" << "Este es el proceso número: " << myRank << endl;
   cout << "Soy la función optimizar";
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

      cout << "\n\n\n\n" << "Antes de condición" << endl;
      if(gen%tamEpoca==0){
          cout << "\n\n\n\n" << "En condición" << endl;
         migracion(newpop);
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
   cout << "\n\n\n\n" << "Voy a hacer unión de poblaciones y soy el " << myRank << endl;
   unionPoblaciones(oldpop);
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

/* Creación aleatoria de la población inicial */
void GeneticoSimple::inicalizarPob()
{
   for (int j = 0; j < popSize; j++)
   {
      oldpop[j].insuflar(problema, precision);
      newpop[j].insuflar(problema, precision);
   }

   if(myRank == RAIZ)
   {
      for (int j = 0; j < (popSize * numIslas); j++)
      {
         globalpop[j].insuflar(problema, precision);
      }
   }


   Pm = 1.0 / oldpop[0].chromoSize;
}

/* Acá deben podrían poner sus métodos nuevos para la versión CONCURRENTE/MPI */
void GeneticoSimple::migracion(Individuo* pop){
   int i;
   cout << "\n\n\n\n" << "Este es el metodo de migración" << endl;
   vector<int> elegidos(nMigrantes);
   obtenElegidos(elegidos,nMigrantes);
   bufSize=nMigrantes*(problema->numVariables()+1)*sizeof(double);
   char* buffer = new char[bufSize];
   int position=0;
   for (i=0; i < nMigrantes; i++) {
      MPI_Pack(pop[elegidos[i]].x.data(), problema->numVariables(), MPI_DOUBLE, buffer, bufSize, 
            &position, MPI_COMM_WORLD);
      cout << "\n\n\n\n" << "\nPosicion bufer antes de envío: " << position << "\n" << endl;
      MPI_Pack(&(pop[elegidos[i]].aptitud), 1, MPI_DOUBLE, buffer, bufSize, 
            &position, MPI_COMM_WORLD);
      cout << "\n\n\n\n" << "\nPosicion bufer antes de envío aptitud: " << position << "\n" << endl;
   }

   cout << "\n\n\n\n" << "\nPosicion antes de envío, después de empaquetado: " << position << "\n" << endl;
   // Imprimir bufSize
   cout << "\n\n\n\n" << "\nbufSize es: " << bufSize << "\n" << "y position es :" << position << endl;

   //Ver si se calcula bien el vacino
   int vecino = myRank + 1;
   if(vecino >= numIslas)
   {
      vecino = 0;
   }
   cout << "\n\n\n\n" << "\nPosicion antes de envío, antes de función: " << position << "en proceso" << myRank << "\n" << endl;
   cout << "\n\n\n\n" << "Antes de enviar paquete, soy " << myRank << "\n" << endl;
   cout << "\n\n\n\n" << "Antes de enviar paquete, mi vecino es  " << vecino << "\n" << endl;
   MPI_Send(buffer, position, MPI_PACKED, vecino, 0, MPI_COMM_WORLD);
   cout << "\n\n\n\n" << "Acabo de enviar paquete, soy " << myRank << "\n" << endl;
   cout << "\n\n\n\n" << "\nPosicion después de envío, antes de función: " << position << "en proceso" << myRank << "\n" << endl;

   int vecino2 = myRank - 1;
   if(vecino2 < 0)
   {
      vecino2 = numIslas - 1;
   }
   //Checar qie position de Recv tiene el mismo valor que el tamaña del buffer
   cout << "\n\n\n\n" << "\nPosicion antes de recibir, antes de función: " << position << "en proceso" << myRank << "\n" << endl;
   cout << "\n\n\n\n" << "Antes de recibir paquete, soy " << myRank << "\n" << endl;
   MPI_Recv(buffer, position, MPI_PACKED, vecino2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
   cout << "\n\n\n\n" << "Acabo de recibir paquete, soy " << myRank << "\n" << endl;
   cout << "\n\n\n\n" << "\nPosicion después de recibir, antes de función: " << position <<  "en proceso" << myRank << "\n" << endl;

   position = 0; 
   for (i = 0; i < nMigrantes; i++) {
      MPI_Unpack(buffer, bufSize, &position, pop[elegidos[i]].x.data(), problema->numVariables(),MPI_DOUBLE, MPI_COMM_WORLD);
      MPI_Unpack(buffer, bufSize, &position, &(pop[elegidos[i]].aptitud), 1,MPI_DOUBLE, MPI_COMM_WORLD);
      pop[elegidos[i]].x2cromosoma(problema); // Obteniene el cromosoma del inmigrante i a partir de x.
   }

   delete[] buffer;
}

void GeneticoSimple::fillArray(int *array){
   int i;
   for(i=0;i<popSize;i++){
      array[i]=i;
   }
}

void GeneticoSimple::obtenElegidos(vector<int>& elegidos, int nMigrantes){
   int array[popSize];
   int decrementarPopSize = popSize - 1;
   fillArray(array);
   int i;
   int numAleatorio;
   for(i=0;i<nMigrantes;i++){
      numAleatorio=rand()%decrementarPopSize;
      //elegidos.push_back(array[numAleatorio]);
      elegidos[i] = array[numAleatorio];
      array[numAleatorio]=array[decrementarPopSize];
      decrementarPopSize--;
   }
}

// Hacer lo que está en las diapositivas 
//a globalpop se accese con globalpop[i] Siempre se ocupa el globalpop.
//Se puede quitar el parametro y trabajar con globalpop directamnete ya que es global
void GeneticoSimple::unionPoblaciones(Individuo* pop){
   //Falta ver cómo calcular tamaño de este buffer
   bufSize=popSize*(problema->numVariables()+2)*sizeof(double);
   char* buffer = new char[bufSize];
   int final;
   if(myRank == RAIZ)
   {
      // La isla 0 debe de copiar a global pop
      for(final = 0; final < popSize; final++)
      {
         globalpop[final] = oldpop[final];
      }
      cout << "Se copió oldpop a globalpop" << endl;
      // Se hace lo de la foto
      int position;
      for(int j = 1; j < numIslas; j++)
      {
         //qué buffer y qué position?
         cout << "\n\n\n\n" << "Soy raíz y estoy recibiendo datos de isla " << j << endl;
         MPI_Recv(buffer, bufSize, MPI_PACKED, j, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
         cout << "\n\n\n\n" << "Soy raíz y recibí datos de " << numIslas << endl;
         //desempacar
         position = 0;
         for(int k = 0; k < popSize; k++)
         {
            //faltan más unpack?
            cout << "\n\n\n\n" << "Soy raiz y voy a desempacar datos de isla" << j << endl;
            MPI_Unpack(buffer, bufSize, &position, globalpop[final].x.data(), problema->numVariables(),MPI_DOUBLE, MPI_COMM_WORLD);
            cout << "\n\n\n\n" << "Soy raiz y desempaqué vector de isla" << j << endl;
            cout << "\n\n\n\n" << "Soy raiz y voy a desempacar eval de isla" << j << endl;
            MPI_Unpack(buffer, bufSize, &position, &(globalpop[final].eval), 1,MPI_DOUBLE, MPI_COMM_WORLD);
            cout << "\n\n\n\n" << "Soy raiz y desempaqué eval de isla" << j << endl;
            cout << "\n\n\n\n" << "Soy raiz y voy a desempacar cons de isla" << j << endl;
            MPI_Unpack(buffer, bufSize, &position, &(globalpop[final].cons[0]), 1,MPI_DOUBLE, MPI_COMM_WORLD);
            cout << "\n\n\n\n" << "Soy raiz y desempaqué cons de isla" << j << endl;
            //hacer copiainal
            final++;
         }
      }
      
      // Para dejar las variables (PESOS) de la población final en este archivo.
      // ***salidafinal*** debe estar donde corren este programa.
      cout << "\n\n\n\n" << "Inicio de escritura de archivos pesos_pob.txt" << endl;
      ofstream archVariables("./salidafinal/pesos_pob.txt", std::ofstream::out);
      cout << "Fin de escritura de archivos pesos_pob.txt" << endl;

      // Para dejar la evaluación (tiempo y distancia restante) de la población.
      cout << "\n\n\n\n" << "Inicio de escritura de archivos evals_pob.txt" << endl;
      ofstream archEvaluacion("./salidafinal/evals_pob.txt", std::ofstream::out);
      cout << "\n\n\n\n" << "Fin de escritura de archivos evals_pob.txt" << endl;

      // Cambiar pop a globalpop y popSize por popSize * numIslas
      cout << "\n\n\n\n" << "Inicio de escritura de variables" << endl;
      stats.writeVariables(archVariables, globalpop, (popSize * numIslas));
      stats.writeEvaluation(archEvaluacion, globalpop, (popSize * numIslas));
      cout << "\n\n\n\n" << "Fin de escritura de variables" << endl;
      archVariables.close();
      archEvaluacion.close();
   }
   
   else{
      //Casi es lo mismo d emigracion que pack y luego send
      int i = 0;
      int position = 0;
      //variable de control está correcta? o debería ser popsize
      cout << "\n\n\n\n" << "Va a empacar proceso " << myRank << endl;
      for (i=0; i < popSize; i++){
         //Enviar esto y aparte x?
         MPI_Pack(oldpop[i].x.data(), problema->numVariables(), MPI_DOUBLE, buffer, bufSize, &position, MPI_COMM_WORLD);
         cout << "\n\n\n\n" << "\nPosicion bufer antes de envío: " << position << "\n" << endl;
         //Empaquetando evaluación de individuo
         MPI_Pack(&(oldpop[i].eval), 1, MPI_DOUBLE, buffer, bufSize, &position, MPI_COMM_WORLD);
         //Empaquetando restricción
         MPI_Pack(&(oldpop[i].cons[0]), 1, MPI_DOUBLE, buffer, bufSize, &position, MPI_COMM_WORLD);
      }
      cout << "\n\n\n\n" << "Terminó de empacar proceso " << myRank << endl;
      //Se envía al proceso 0
      cout << "Voy a enviar en migración y soy el proceso " << myRank << endl;
      MPI_Send(buffer, position, MPI_PACKED, RAIZ, 0, MPI_COMM_WORLD);
   }
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
