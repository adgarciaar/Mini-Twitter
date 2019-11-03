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

int fd1;
char mensaje[TAMMENSAJE];
char pipeEspecifico[25] = "pipeCliente";
bool clienteAceptado;
int pidServidor;

typedef void (*sighandler_t)(int);

sighandler_t signalHandler (void){

    int numero_bytes;
    int creado;
    char mensaje[200];
    mensajeDelServidor mensajeRecibido;

    do {
       fd1 = open(pipeEspecifico, O_RDONLY | O_NONBLOCK);
       if (fd1 == -1) {
           perror("pipe");
           printf(" Se volvera a intentar despues\n");
           sleep(10);
       } else creado = 1;
    } while (creado == 0);

    printf("\n\n--------------Mensaje recibido desde el servidor--------------\n\n");

    numero_bytes = read (fd1, &mensajeRecibido, sizeof(mensajeRecibido) );
    if (numero_bytes == -1){
        perror("proceso servidor: ");
        exit(1);
    }

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
        unlink(pipeEspecifico);
        printf("Desconexion realizada\n");
        exit(1);
    }else{
        clienteAceptado = true;
    }

}

int main (int argc, char **argv){

    int  fd, pid, creado = 0, res;
    infoPipe datosProcesoCliente;
    int opcion = 0;
    int numero_bytes;
    int confirmacionSenal;

    int idCliente;
    char* pipeInicial;

    mode_t fifo_mode = S_IRUSR | S_IWUSR;

    mensajeDelCliente mensaje_a_enviar;

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

    /* Se abre el pipe cuyo nombre se recibe como argumento del main. */
    do {
       fd = open(pipeInicial, O_WRONLY|O_NONBLOCK);
       if (fd == -1) {
           perror("pipe");
           printf(" Se volvera a intentar despues\n");
  	       sleep(10);
       } else creado = 1;
    } while (creado == 0);

    pid = getpid();
    char stringPidProceso[10];
    sprintf(stringPidProceso, "%d", pid);
    strcat(pipeEspecifico, stringPidProceso);

    datosProcesoCliente.pid = pid;
    strcpy(datosProcesoCliente.pipeEspecifico, pipeEspecifico);
    datosProcesoCliente.numeroCliente = idCliente;

    /*Se crea un pipe específico para la comunicación con el server.*/
    unlink(pipeEspecifico);
    if (mkfifo (datosProcesoCliente.pipeEspecifico, fifo_mode) == -1) {
       perror("Client  mkfifo");
       exit(1);
    }
    printf("\nPipe especifico creado: %s\n", pipeEspecifico);

    /*Se abre el pipe específicos*/
   do {
      fd1 = open(pipeEspecifico, O_RDONLY | O_NONBLOCK);
      if (fd1 == -1) {
          perror("pipe");
          printf(" Se volvera a intentar despues\n");
          sleep(10);
      } else creado = 1;
   } while (creado == 0);

    /* se envia el nombre del pipe al otro proceso. */
    write(fd, &datosProcesoCliente , sizeof(datosProcesoCliente));

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

                creado = 0;
                do {
                   fd1 = open(pipeEspecifico, O_WRONLY | O_NONBLOCK);
                   if (fd1 == -1) {
                       perror("pipe");
                       printf(" Se volvera a intentar despues\n");
                       sleep(10);
                   } else creado = 1;
                } while (creado == 0);

                switch (opcion){
                    case 1:
                        printf("Ejecutar follow");
                        break;
                    case 2:
                        printf("Ejecutar unfollow");
                        break;
                    case 3:
                        printf("Ejecutar tweet");

                        mensaje_a_enviar.pid = getpid();
                        mensaje_a_enviar.desconexion = false;
                        mensaje_a_enviar.numeroCliente = idCliente;
                        strcpy(mensaje_a_enviar.mensaje , "Quiero jugar Apex Legends\n");

                        write(fd1, &mensaje_a_enviar , sizeof(mensaje_a_enviar));
                        printf("Tweet enviado al servidor\n");
                        confirmacionSenal = kill (pidServidor, SIGUSR1); /*enviar la señal*/
                        if(confirmacionSenal == -1){
                            perror("No se pudo enviar señal");
                        }

                        break;
                }
            }
        }

    } while (opcion != 4);

    mensaje_a_enviar.pid = getpid();
    mensaje_a_enviar.desconexion = true;
    mensaje_a_enviar.numeroCliente = idCliente;
    strcpy(mensaje_a_enviar.mensaje , "");

    write(fd1, &mensaje_a_enviar , sizeof(mensaje_a_enviar));
    printf("Desconexion avisada al servidor\n");
    confirmacionSenal = kill (pidServidor, SIGUSR1); /*enviar la señal*/
    if(confirmacionSenal == -1){
        perror("No se pudo enviar señal");
    }

    sleep(2);

    unlink(pipeEspecifico);

    printf("Desconexion realizada\n");

    exit(0);
}
