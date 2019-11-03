/*
Autor:
Funcion:
Nota:
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "nom.h"

#define TAMNOM 200

int numeroUsuarios;
estadosClientes* clientesEstados;

typedef void (*sighandler_t)(int);

sighandler_t signalHandler (void){

    int i, pipeConPaquete, creado = 0;
    int numero_bytes;
    mensajeDelCliente mensajeRecibido;

    printf("\n\nRecibiendo un paquete de un cliente");

    for(i=0;i<numeroUsuarios;i++){

        if ( strcmp(clientesEstados[i].pipeEspecifico,"") != 0 ){

            /*printf("el especifico revisandose es %s\n", clientesEstados[i].pipeEspecifico);*/

            creado = 0;
            do {
               if ((pipeConPaquete = open(clientesEstados[i].pipeEspecifico, O_RDONLY | O_NONBLOCK)) == -1) {
                  perror("\nServer abriendo el pipe especifico\n");
                  printf("Se volvera a intentar despues\n");
                  sleep(5);
               } else creado = 1;
            }  while (creado == 0);

            numero_bytes = read (pipeConPaquete, &mensajeRecibido, sizeof(mensajeRecibido) );
            /*if (numero_bytes == -1){
                perror("proceso servidor: ");
                /*exit(1);
            }*/
            if(numero_bytes > 0){
                printf("Recibido paquete del cliente con id %d y pid %d\n",  mensajeRecibido.numeroCliente,
                    mensajeRecibido.pid);
                if(mensajeRecibido.desconexion == true){
                    clientesEstados[i].activo = false;
                    clientesEstados[i].pid = -1;
                    strcpy(clientesEstados[i].pipeEspecifico , "");
                    printf("Se desconecta el cliente con id %d y pid %d\n",  mensajeRecibido.numeroCliente,
                        mensajeRecibido.pid);
                }else{
                    printf("Tweet enviado: %s\n", mensajeRecibido.mensaje);
                }
            }
            close(pipeConPaquete);
        }

    }

  /*  printf("\n\nRecibiendo un paquete de cliente con id %d y pid %d\n", nuevoCliente.numeroCliente, nuevoCliente.pid);*/


}


void manejarNuevaConexion(infoPipe nuevoCliente){

    printf("Enviando respuesta inicial a cliente con id %d y pid %d\n", nuevoCliente.numeroCliente, nuevoCliente.pid);

    int fd1, creado = 0, confirmacionSenal;

    do {
       if ((fd1 = open(nuevoCliente.pipeEspecifico, O_WRONLY| O_NONBLOCK)) == -1) {
          perror("\nServer abriendo el pipe especifico\n");
          printf("Se volvera a intentar despues\n");
          sleep(5);
       } else creado = 1;
    }  while (creado == 0);

    mensajeDelServidor mensajeParaCliente;

    mensajeParaCliente.pid = getpid();

    if (nuevoCliente.numeroCliente < 1 || nuevoCliente.numeroCliente>10){
        mensajeParaCliente.numeroMensajes = -1;
    }else{
        mensajeParaCliente.numeroMensajes = 0;
        clientesEstados[ nuevoCliente.numeroCliente - 1 ].pid = nuevoCliente.pid;
        strcpy(clientesEstados[ nuevoCliente.numeroCliente - 1 ].pipeEspecifico , nuevoCliente.pipeEspecifico);
        clientesEstados[ nuevoCliente.numeroCliente - 1 ].activo = true;
    }

    write(fd1, &mensajeParaCliente, sizeof(mensajeParaCliente) );

    confirmacionSenal = kill (nuevoCliente.pid, SIGUSR1); /*enviar la señal*/
    if(confirmacionSenal == -1){
        perror("No se pudo enviar señal");
    }

    printf("Enviada respuesta inicial a cliente con id %d y pid %d\n\n", nuevoCliente.numeroCliente, nuevoCliente.pid);
}

int main (int argc, char **argv){

    int  fd, fd1,  pid, n, cuantos,res,creado=0,i;
    infoPipe datosProcesoCliente;

    mode_t fifo_mode = S_IRUSR | S_IWUSR;

    numeroUsuarios = 10;

    clientesEstados = (estadosClientes*)malloc(numeroUsuarios*sizeof(estadosClientes));
    if (clientesEstados == NULL) {
        perror("Memoria no alocada");
        exit(1);
    }/*end if*/

    for(i=0;i<numeroUsuarios;i++){
        clientesEstados[i].pid = 0;
        clientesEstados[i].activo = false;
        strcpy(clientesEstados[i].pipeEspecifico , "");
    }

    /* Instalar manejador de la señal */
    signal (SIGUSR1, (sighandler_t)signalHandler);

    /*if(argc!=5){
          /*No pueden haber más ni menos de 5 argumentos (máximo)
          printf("Error con numero de argumentos\n");
          printf("%s\n", "Explicar la vaina al usuario");
          exit(1);
    }/*end if*/

    /* Creacion del pipe inicial, el que se recibe como argumento del main */
    unlink(argv[1]);
    if (mkfifo (argv[1], fifo_mode) == -1) {
       perror("Server mkfifo");
       exit(1);
    }

    do {
       fd = open (argv[1], O_RDONLY);
       if (fd == -1) {
           perror("Pipe: ");
           printf(" Se volvera a intentar despues\n");
           sleep(5);
       } else creado = 1;
    } while (creado == 0);

    int numeroClienteConectado;
    int numero_bytes;

    while(1){

        numero_bytes = read (fd, &datosProcesoCliente, sizeof(infoPipe) );
        if (numero_bytes == -1){
            perror("proceso servidor: ");
            exit(1);
        }

        if(numero_bytes > 0){

            printf("\n\nSe conectó un cliente con id %d y pid %d\n", datosProcesoCliente.numeroCliente
              ,datosProcesoCliente.pid);
            printf("Comunicación con cliente con id %d y pid %d se hará a través de %s\n",
              datosProcesoCliente.numeroCliente,datosProcesoCliente.pid,
              datosProcesoCliente.pipeEspecifico);
            manejarNuevaConexion(datosProcesoCliente);

        }
    }

    exit(0);

}
