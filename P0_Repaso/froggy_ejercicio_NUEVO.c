#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define CARRIL_INICIO 5
#define COL_SALIDA 10
#define COL_META 80
#define MIN_DELAY    10000  //0.01 sec
#define MAX_DELAY  100000  //1.0 sec
#define MAX_FROGS 16

char emojis[MAX_FROGS][15] = {"🐸"};
// Acá abajo hay 16 personajes ("ranas")
//char emojis[MAX_FROGS][15] = {"🐸", "🐺", "🐌","🐯", "🦑", "🦖", "🐷", "🦇", "🐮", "🦏", "🐰", "🐥", "🐨", "👾", "🧠", "🦀"};


typedef struct {
    int ren; // renglón actual de la partícula
    int col; // columna actual de la partícula
    int delay;  // cuánto tardará en avanzar un paso.
} Rana;

// Una enumeración asigna un alias a los códigos enteros de los colores.
enum COLORES {NEGRO=30, ROJO, VERDE, AMARILLO, AZUL, MAGENTA, CYAN, BLANCO};

/**** Esta es la función para los hilos. ****/
void moverRana(void* ptr);
/********************************************/


// Estas son funciones auxiliares
void escribirR_C(int ren, int col, char* texto, int color);
Rana crearRana();


// 3. TERCER cambio: registrar qué rana fue la ganadora.
// Para esto necesitamos dos variables globales.
//  a) una para saber si alguien ya ganó (bandera booleana)
//  b) un entero para guardar el id de la rana ganadora.

bool hayGanador = false;
int idGanador;

int main(int argc, char const *argv[])
{
   
    // Verificar que nos hayan pasado el número de partículas.
    if (argc < 2) {
        printf("\nUso:\n");
        printf("%s <núm. ranas (1--%d)>\n\n", argv[0], MAX_FROGS);
        return 0;
    }    

    srand(time(NULL));

    // Convertir la cadena del núm. de partículas a entero.
    int nRanas;
    sscanf(argv[1], "%d", &nRanas);

    if (nRanas > MAX_FROGS) {
       printf("\nEl número máximo de ranas es %d\n\n", MAX_FROGS);
       return 0;
    }

    
    printf("\033[2J");   // Limpiar la pantalla de la consola. 
    printf("\033[?25l"); // Hacer INVISIBLE el cursor del texto.
    escribirR_C(CARRIL_INICIO-1, COL_META, "🏁", ROJO);
    escribirR_C(CARRIL_INICIO+nRanas, COL_META, "🏁", ROJO);

    // Arreglo de hilos para poder crear varias Ranas
    pthread_t hilos[nRanas];

    // Arreglo para tener identificadores de las ranas.  
    int idRanas[nRanas];

    // 1. PRIMER CAMBIO: pasar el id de la rana como argumento al crear el hilo.
    // 2. SEGUNDO CAMBIO: poner un ciclo para crear varias ranas. 
    // La línea de abajo se debe sustituir por un ciclo for para
    // crear nRanas hilos. 

 
    idRanas[0] = 0;
    pthread_create(&hilos[0], NULL, (void *) moverRana, &idRanas[0]);


    // 3. TERCER CAMBIO: esperar a todas la ranas. 
    // De manera similar, acá deben poner un ciclo para
    // esperar a que terminen todas las ranas.
    pthread_join(hilos[0], NULL);
    
    printf("\033[?25h"); // Hacer VISIBLE el cursor del texto.

    return 0;
}

void moverRana(void* ptrID) {
    // ¿Cómo pueden pasarle un ID diferente a cada rana/hilo?
    int* ptrInt = (int *) ptrID;
    int miID = *ptrInt;
    
    Rana p = crearRana(miID);

    escribirR_C(p.ren, p.col, emojis[miID], ROJO);

    while (p.col < COL_META) 
    {
        usleep(MIN_DELAY + rand() % MAX_DELAY);
        escribirR_C(p.ren, p.col, " ", ROJO);
        p.col = p.col + 1;
        escribirR_C(p.ren, p.col, emojis[miID], ROJO);
    }

    // 3. TERCER cambio: registrar qué rana fue la ganadora.
    // AQUÍ DEBERÍAN PREGUNTAR SI NO HAY GANADOR AÚN
    // SI NO HAY GANADOR ENTONCES EL GANADOR SOY YO (miID)
    // Y DEBO INDICAR QUE YA HAY GANADOR

    printf("\n\nFIN\n\n");
}

// Inciar los parámetros de la partícula de manera aleatoria.
Rana crearRana(int id) {
    Rana p;
    p.ren = CARRIL_INICIO + id;  // los renglones comienzan en 1.
    p.col = COL_SALIDA;  // las columnas comienzan en 1.
    p.delay = MIN_DELAY + rand() % MAX_DELAY;

    return p;
}

void escribirR_C(int ren, int col, char* texto, int color) {
    printf("\033[38;5;%dm", color);
    printf("\033[%d;%dH", ren, col);
    printf("%s", texto);
    fflush(stdout);
}
