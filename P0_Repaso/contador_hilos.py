import threading
import time 

# Este contador global es COMPARTIDO por todos los hilos (main y nuevo).
contador = 0;

# Esta funcion es para que el hilo la ejecute. 
def muestraContador(): 
   while True:
      print("\n Valor actual del contador: ", contador, "\n", flush=True)
      time.sleep(1)


if __name__ == "__main__":

    # Comienza solamente un hilo. 
    
    # El hilo principal inicializa el contador.
    contador = 0
        
    # Crear un hilo para mostrar períodicamente el valor del contador.
    hilo = threading.Thread(target=muestraContador, daemon=True)
    hilo.start() # En Python tenemos que indicar cuando arrancará el hilo.
     
    # ESTO YA NO LO EJECUTA EL HILO NUEVO. 
 
    # Incrementar el contador LENTAMENTE.     
    for i in range(5):
       contador += 1
       time.sleep(2)
