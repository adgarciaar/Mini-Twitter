/*
Autor:
Función:
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

#define TAMMENSAJE 200

int id_pipe_cliente_a_servidor, id_pipe_servidor_a_cliente;
char mensaje[TAMMENSAJE];
char pipe_cliente_a_servidor[30] = "pipeCliente";
char pipe_servidor_a_cliente[30] = "pipeCliente";
bool clienteAceptado;
int pidServidor;
int idCliente;

typedef void (*sighandler_t)(int);

sighandler_t signalHandler (void){

    int numero_bytes;
    int creado;
    char mensaje[200];
    mensajeDelServidor mensajeRecibido;

    do {
       id_pipe_servidor_a_cliente = open(pipe_servidor_a_cliente, O_RDONLY | O_NONBLOCK);
       if (id_pipe_servidor_a_cliente == -1) {
           perror("pipe");
           printf(" Se volverá a intentar después\n");
           sleep(10);
       } else creado = 1;
    } while (creado == 0);

    numero_bytes = read (id_pipe_servidor_a_cliente, &mensajeRecibido, sizeof(mensajeRecibido) );
    if (numero_bytes == -1){
        perror("proceso servidor: ");
        exit(1);
    }

    printf("\n\n--------------Mensaje recibido desde el servidor--------------\n\n");

    pidServidor = mensajeRecibido.pid;

    if(mensajeRecibido.numeroMensajes == -2){
        printf("El id del cliente no es válido, ya se encuentra conectado. Se procede a desconectar.");
    }

    if(mensajeRecibido.numeroMensajes == -1){
        printf("El id del cliente no es válido, no existe dentro del registro. Se procede a desconectar.");
    }
    if(mensajeRecibido.numeroMensajes == 0){
        printf("El id del cliente es válido");
    }
    printf("\n\n--------------Mensaje recibido desde el servidor--------------\n\n");
    if(mensajeRecibido.numeroMensajes < 0){
        unlink(pipe_cliente_a_servidor);
        unlink(pipe_servidor_a_cliente);
        printf("Desconexion realizada\n");
        exit(1);
    }else{
        clienteAceptado = true;
    }

}

void follow(){

}

void unfollow(){

}

void enviarTweet(){

    int confirmacionSenal;
    mensajeDelCliente mensaje_a_enviar;
    char tweet[201];
    int creado = 0;
    int c;
    int contador = 0;
    /*char* p;*/

    printf("Digite el tweet:\n");
    /*scanf(" %[^\n]", tweet);*/
    scanf(" %200[^\n]s", tweet);
    strcat(tweet, "\0");

    while ((c = fgetc(stdin)) != '\n' && c != EOF){ /* Flush stdin */
        contador = contador+1; //sumar cada caracter que sobró y quedó en stdin
    }

    if(contador > 0){
        printf("Error: el tweet supera los 200 caracteres\n");
    }else{
        mensaje_a_enviar.pid = getpid();
        mensaje_a_enviar.operacion = 3;
        mensaje_a_enviar.numeroCliente = idCliente;
        strcpy(mensaje_a_enviar.mensaje , tweet);

        do {
           id_pipe_cliente_a_servidor = open(pipe_cliente_a_servidor, O_WRONLY | O_NONBLOCK);
           if (id_pipe_cliente_a_servidor == -1) {
               perror("pipe");
               printf(" Se volvera a intentar despues\n");
               sleep(10);
           } else creado = 1;
        } while (creado == 0);

        write(id_pipe_cliente_a_servidor, &mensaje_a_enviar , sizeof(mensajeDelCliente));

        printf("Tweet enviado al servidor\n");
        confirmacionSenal = kill (pidServidor, SIGUSR1); /*enviar la señal*/
        if(confirmacionSenal == -1){
            perror("No se pudo enviar señal");
        }
    }
}

void desconectar(){

    int confirmacionSenal;
    int creado = 0;
    mensajeDelCliente mensaje_a_enviar;

    mensaje_a_enviar.pid = getpid();
    mensaje_a_enviar.operacion = 4;
    mensaje_a_enviar.numeroCliente = idCliente;

    do {
       id_pipe_cliente_a_servidor = open(pipe_cliente_a_servidor, O_WRONLY | O_NONBLOCK);
       if (id_pipe_cliente_a_servidor == -1) {
           perror("pipe");
           printf(" Se volvera a intentar despues\n");
           sleep(10);
       } else creado = 1;
    } while (creado == 0);

    write(id_pipe_cliente_a_servidor, &mensaje_a_enviar , sizeof(mensajeDelCliente));

    printf("Desconexion avisada al servidor\n");
    confirmacionSenal = kill (pidServidor, SIGUSR1); /*enviar la señal*/
    if(confirmacionSenal == -1){
        perror("No se pudo enviar señal");
    }
}

int main (int argc, char **argv){

    int id_pipe_inicial, pid, creado = 0, res;
    comunicacionInicialCliente datosProcesoCliente;
    int opcion = 0;
    int numero_bytes;
    int status, c;
    char cOpcion;
    int contador;

    char* pipeInicial;

    mode_t fifo_mode = S_IRUSR | S_IWUSR;

    clienteAceptado = false;

    /* Instalar manejador de la señal */
    signal (SIGUSR1, (sighandler_t)signalHandler);

    if(argc!=5){
          /*No pueden haber más ni menos de 5 argumentos (máximo)*/
          printf("Error con numero de argumentos\n");
          printf("%s\n", "Explicar la vaina al usuario");
          exit(1);
    }/*end if*/

    idCliente = atoi(argv[2]);
    pipeInicial = argv[4];

    pid = getpid();
    char stringPidProceso[10];
    sprintf(stringPidProceso, "%d", pid);

    strcat(pipe_cliente_a_servidor, stringPidProceso);
    strcat(pipe_cliente_a_servidor, "_c_a_s");
    strcat(pipe_servidor_a_cliente, stringPidProceso);
    strcat(pipe_servidor_a_cliente, "_s_a_c");

    datosProcesoCliente.pid = pid;
    strcpy(datosProcesoCliente.pipe_cliente_a_servidor, pipe_cliente_a_servidor);
    strcpy(datosProcesoCliente.pipe_servidor_a_cliente, pipe_servidor_a_cliente);
    datosProcesoCliente.numeroCliente = idCliente;

    /*Se crea un pipe específico para la comunicación con el server.*/
    unlink(pipe_cliente_a_servidor);
    if (mkfifo (pipe_cliente_a_servidor, fifo_mode) == -1) {
       perror("Client  mkfifo");
       exit(1);
    }
    printf("\nPipe especifico creado: %s\n", pipe_cliente_a_servidor);

    /*Se crea un pipe específico para la comunicación con el server.*/
    unlink(pipe_servidor_a_cliente);
    if (mkfifo (pipe_servidor_a_cliente, fifo_mode) == -1) {
       perror("Client  mkfifo");
       exit(1);
    }
    printf("\nPipe especifico creado: %s\n", pipe_servidor_a_cliente);

    /*Se abre el pipe del servidor al cliente*/
    creado = 0;
     do {
        id_pipe_servidor_a_cliente = open(pipe_servidor_a_cliente, O_RDONLY | O_NONBLOCK);
        if (id_pipe_servidor_a_cliente == -1) {
            perror("pipe");
            printf(" Se volvera a intentar despues\n");
            sleep(10);
        } else creado = 1;
     } while (creado == 0);

     /* Se abre el pipe cuyo nombre se recibe como argumento del main. */
     do {
        id_pipe_inicial = open(pipeInicial, O_WRONLY|O_NONBLOCK);
        if (id_pipe_inicial == -1) {
            perror("pipe");
            printf(" Se volvera a intentar despues\n");
   	       sleep(10);
        } else creado = 1;
     } while (creado == 0);

    /* se envia la información del cliente al servidor */
    write(id_pipe_inicial, &datosProcesoCliente , sizeof(comunicacionInicialCliente));

    do{
        if ( clienteAceptado == true ){

            printf("\n\nOPCIONES\n\n");
            printf("Presione 1 para follow\n");
            printf("Presione 2 para unfollow\n");
            printf("Presione 3 para tweet\n");
            printf("Presione 4 para desconexion\n");
            printf("Digite numero de la opcion: ");

            status = scanf("%c", &cOpcion);
            while ((c = fgetc(stdin)) != '\n' && c != EOF){
                contador = contador + 1;
            }; /* Flush stdin */

            while(contador>0 || (int)cOpcion <49 || (int)cOpcion>52){
                contador = 0;
                printf("Error: por favor ingrese un número válido\n");
                printf("Digite numero de la opcion: ");
		            status = scanf("%c", &cOpcion);
                while ((c = fgetc(stdin)) != '\n' && c != EOF){
                    contador = contador + 1;
                };
            }

            /*opcion = (int)cOpcion;*/

            switch ( (int)cOpcion ){
                case 49:
                    opcion = 1;
                    break;
                case 50:
                    opcion = 2;
                    break;
                case 51:
                    opcion = 3;
                    break;
                case 52:
                    opcion = 4;
                    break;
            }

            printf("Digitado fue %d\n", opcion);

            /*if(opcion < 49 || opcion > 52){
                printf("Opcion es incorrecta");
            }*/

            if(opcion >= 1 && opcion <=4){

                switch (opcion){
                    case 1:
                        follow();
                        break;
                    case 2:
                        unfollow();
                        break;
                    case 3:
                        enviarTweet();
                        break;
                }
            }
        }

    } while (opcion != 4);

    desconectar();
    sleep(1);
    unlink(pipe_cliente_a_servidor);
    unlink(pipe_servidor_a_cliente);
    printf("Desconexion realizada\n");
    exit(0);
}
