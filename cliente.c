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
    /*printf("%s", mensajeRecibido.mensaje);*/

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
    char tweet[200];
    int creado = 0;

    printf("Digite el tweet:\n");
    scanf(" %[^\n]", tweet);

    mensaje_a_enviar.pid = getpid();
    mensaje_a_enviar.desconexion = 0;
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

void desconectar(){

    int confirmacionSenal;
    int creado = 0;
    mensajeDelCliente mensaje_a_enviar;

    mensaje_a_enviar.pid = getpid();
    mensaje_a_enviar.desconexion = 1;
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

    /* se envia el nombre del pipe al otro proceso. */
    write(id_pipe_inicial, &datosProcesoCliente , sizeof(comunicacionInicialCliente));

    /*pause();*/

    do{
        if ( clienteAceptado == true ){

            printf("\n\nOPCIONES\n\n");
            printf("Presione 1 para follow\n");
            printf("Presione 2 para unfollow\n");
            printf("Presione 3 para tweet\n");
            printf("Presione 4 para desconexion\n");
            printf("Digite numero de la opcion: \n");
            scanf("%d", &opcion);

            if(opcion < 1 || opcion > 4){
                printf("Opcion es incorrecta");
            }

            if(opcion >= 1 && opcion <=3){

                switch (opcion){
                    case 1:
                        printf("Ejecutar follow");
                        follow();
                        break;
                    case 2:
                        printf("Ejecutar unfollow");
                        unfollow();
                        break;
                    case 3:
                        printf("Ejecutar tweet");
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
