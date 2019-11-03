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

    int i, id_pipe_cliente_a_servidor, creado = 0;
    int numero_bytes;
    mensajeDelCliente mensajeRecibido;

    printf("\n\nRecibiendo un paquete de un cliente");

    for(i=0;i<numeroUsuarios;i++){

        if ( clientesEstados[i].activo == true ){

            creado = 0;
            do {
               id_pipe_cliente_a_servidor = open(clientesEstados[i].pipe_cliente_a_servidor, O_RDONLY | O_NONBLOCK);
               if (id_pipe_cliente_a_servidor == -1) {
                  perror("\nServer abriendo el pipe especifico\n");
                  printf("Se volvera a intentar despues\n");
                  sleep(5);
               } else creado = 1;
            }  while (creado == 0);

            numero_bytes = read (id_pipe_cliente_a_servidor, &mensajeRecibido, sizeof(mensajeDelCliente) );
            /*if (numero_bytes == -1){
                perror("proceso servidor: ");
                /*exit(1);
            }*/
            if(numero_bytes==0){
              printf("\nel especifico revisandose es %s\n", clientesEstados[i].pipe_cliente_a_servidor);
            }
            if(numero_bytes > 0){
                printf("Recibido paquete del cliente con id %d y pid %d\n",  mensajeRecibido.numeroCliente,
                    mensajeRecibido.pid);

                if(mensajeRecibido.desconexion == 1){

                    clientesEstados[i].activo = false;
                    clientesEstados[i].pid = -1;
                    strcpy(clientesEstados[i].pipe_cliente_a_servidor , "");
                    strcpy(clientesEstados[i].pipe_servidor_a_cliente , "");
                    printf("Se desconecta el cliente con id %d y pid %d\n\n",  mensajeRecibido.numeroCliente,
                        mensajeRecibido.pid);

                }else{
                    printf("Tweet recibido: %s\n", mensajeRecibido.mensaje);
                }
            }

            /*close(pipeConPaquete);*/
        }

    }

  /*  printf("\n\nRecibiendo un paquete de cliente con id %d y pid %d\n", nuevoCliente.numeroCliente, nuevoCliente.pid);*/


}


void manejarNuevaConexion(comunicacionInicialCliente nuevoCliente){

    int id_pipe_servidor_a_cliente, creado = 0, confirmacionSenal, id_pipe_cliente_a_servidor;

    printf("Enviando respuesta inicial a cliente con id %d y pid %d\n", nuevoCliente.numeroCliente, nuevoCliente.pid);

    mensajeDelServidor mensajeParaCliente;

    mensajeParaCliente.pid = getpid();

    if (nuevoCliente.numeroCliente < 1 || nuevoCliente.numeroCliente>10){
        mensajeParaCliente.numeroMensajes = -1;
    }else{
        if( clientesEstados[ nuevoCliente.numeroCliente - 1 ].activo == true ){
          //si ese cliente ya está conectado entonces esta nueva conexión no es válida
            mensajeParaCliente.numeroMensajes = -2;
        }else{
            mensajeParaCliente.numeroMensajes = 0;
            clientesEstados[ nuevoCliente.numeroCliente - 1 ].pid = nuevoCliente.pid;
            strcpy(clientesEstados[ nuevoCliente.numeroCliente - 1 ].pipe_cliente_a_servidor , nuevoCliente.pipe_cliente_a_servidor);
            strcpy(clientesEstados[ nuevoCliente.numeroCliente - 1 ].pipe_servidor_a_cliente , nuevoCliente.pipe_servidor_a_cliente);
            clientesEstados[ nuevoCliente.numeroCliente - 1 ].activo = true;

            creado = 0;
            do {
               id_pipe_cliente_a_servidor = open(nuevoCliente.pipe_cliente_a_servidor, O_RDONLY | O_NONBLOCK);
               if (id_pipe_cliente_a_servidor == -1) {
                  perror("\nServer abriendo el pipe especifico\n");
                  printf("Se volvera a intentar despues\n");
                  sleep(5);
               } else creado = 1;
            }  while (creado == 0);
        }
    }

    do {
       id_pipe_servidor_a_cliente = open(nuevoCliente.pipe_servidor_a_cliente, O_WRONLY| O_NONBLOCK);
       if ( id_pipe_servidor_a_cliente == -1 ) {
          perror("\nServer abriendo el pipe especifico\n");
          printf("Se volvera a intentar despues\n");
          sleep(5);
       } else creado = 1;
    }  while (creado == 0);

    write(id_pipe_servidor_a_cliente, &mensajeParaCliente, sizeof(mensajeDelServidor) );

    /*close(fd1);*/

    confirmacionSenal = kill (nuevoCliente.pid, SIGUSR1); /*enviar la señal*/
    if(confirmacionSenal == -1){
        perror("No se pudo enviar señal");
    }

    printf("Enviada respuesta inicial a cliente con id %d y pid %d\n\n", nuevoCliente.numeroCliente, nuevoCliente.pid);
}

int main (int argc, char **argv){

    int id_pipe_inicial, fd1,  pid, n, cuantos,res,creado=0,i;
    comunicacionInicialCliente datosProcesoCliente;

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
        strcpy(clientesEstados[i].pipe_cliente_a_servidor , "");
        strcpy(clientesEstados[i].pipe_servidor_a_cliente , "");
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

    int numeroClienteConectado;
    int numero_bytes;

    while(1){

        creado = 0;
        do {
           id_pipe_inicial = open (argv[1], O_RDONLY);
           if (id_pipe_inicial == -1) {
               perror("Pipe: ");
               printf(" Se volvera a intentar despues\n");
               sleep(5);
           } else creado = 1;
        } while (creado == 0);

        numero_bytes = read (id_pipe_inicial, &datosProcesoCliente, sizeof(comunicacionInicialCliente) );
        if (numero_bytes == -1){
            perror("proceso servidor: ");
            exit(1);
        }

        if(numero_bytes > 0){

            printf("\n\nSe conectó un cliente con id %d y pid %d\n", datosProcesoCliente.numeroCliente
              ,datosProcesoCliente.pid);
            printf("Comunicación con cliente con id %d y pid %d se hará a través de %s y %s\n",
              datosProcesoCliente.numeroCliente,datosProcesoCliente.pid,
              datosProcesoCliente.pipe_cliente_a_servidor,
              datosProcesoCliente.pipe_servidor_a_cliente);
            manejarNuevaConexion(datosProcesoCliente);

        }
    }

    exit(0);

}
