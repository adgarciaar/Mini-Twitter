/*
Autor:
Función:
Nota:
*/

#include "cliente.h"

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
    if(mensajeRecibido.numeroMensajes == 1){
        switch (mensajeRecibido.operacion) {
          case 1: /*follow*/
              printf("Respuesta de operación follow:\n %s", mensajeRecibido.mensaje);
              break;
          case 2: /*unfollow*/
              printf("Respuesta de operación unfollow:\n %s", mensajeRecibido.mensaje);
              break;
          case 3: /*tweet*/
              printf("Tweet realizado por cliente %d\n",mensajeRecibido.idTweetero);
              printf("Tweet: %s\n", mensajeRecibido.mensaje);
              break;
        }
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

    int confirmacionSenal;
    mensajeDelCliente mensaje_a_enviar;
    int creado = 0, status, c, contador=0;
    int usuario_a_seguir;

    printf("Por favor ingrese el número del usuario a seguir: ");

    status = scanf("%d", &usuario_a_seguir);
    while ((c = fgetc(stdin)) != '\n' && c != EOF){
        contador = contador + 1;
    }; /* Flush stdin */

    while(contador>0 || status!=1 || usuario_a_seguir <= 0 || usuario_a_seguir == idCliente){
        contador = 0;
        if( usuario_a_seguir <= 0 ){
            printf("Error: el número debe ser mayor a cero\n");
        }
        if( usuario_a_seguir == idCliente ){
            printf("Error: no te puedes seguir a ti mismo\n");
        }
        if (contador>0 || status!=1){
          printf("Error: por favor ingrese un número\n");
        }
        printf("Por favor ingrese el número del usuario a seguir: ");
        status = scanf("%d", &usuario_a_seguir);
        while ((c = fgetc(stdin)) != '\n' && c != EOF){
            contador = contador + 1;
        }
    }

    mensaje_a_enviar.pid = getpid();
    mensaje_a_enviar.operacion = 1;
    mensaje_a_enviar.numeroCliente = idCliente;
    mensaje_a_enviar.numero_cliente_follow_unfollow = usuario_a_seguir;

    do {
       id_pipe_cliente_a_servidor = open(pipe_cliente_a_servidor, O_WRONLY | O_NONBLOCK);
       if (id_pipe_cliente_a_servidor == -1) {
           perror("pipe");
           printf(" Se volvera a intentar despues\n");
           sleep(10);
       } else creado = 1;
    } while (creado == 0);

    write(id_pipe_cliente_a_servidor, &mensaje_a_enviar , sizeof(mensajeDelCliente));

    printf("Solicitud de follow enviada al servidor\n");
    confirmacionSenal = kill (pidServidor, SIGUSR1); /*enviar la señal*/
    if(confirmacionSenal == -1){
        perror("No se pudo enviar señal");
    }

}

void unfollow(){

    int confirmacionSenal;
    mensajeDelCliente mensaje_a_enviar;
    int creado = 0, status, c, contador=0;
    int usuario_a_dejar_de_seguir;

    printf("Por favor ingrese el número del usuario a dejar de seguir: ");

    status = scanf("%d", &usuario_a_dejar_de_seguir);
    while ((c = fgetc(stdin)) != '\n' && c != EOF){
        contador = contador + 1;
    }; /* Flush stdin */

    while(contador>0 || status!=1 || usuario_a_dejar_de_seguir <= 0 || usuario_a_dejar_de_seguir == idCliente){
        contador = 0;
        if( usuario_a_dejar_de_seguir <= 0 ){
            printf("Error: el número debe ser mayor a cero\n");
        }
        if( usuario_a_dejar_de_seguir == idCliente ){
            printf("Error: no te puedes dejar de seguir a ti mismo (porque no te puedes seguir a ti mismo)\n");
        }
        if (contador>0 || status!=1){
          printf("Error: por favor ingrese un número\n");
        }
        printf("Por favor ingrese el número del usuario a dejar de seguir: ");
        status = scanf("%d", &usuario_a_dejar_de_seguir);
        while ((c = fgetc(stdin)) != '\n' && c != EOF){
            contador = contador + 1;
        }
    }

    mensaje_a_enviar.pid = getpid();
    mensaje_a_enviar.operacion = 2;
    mensaje_a_enviar.numeroCliente = idCliente;
    mensaje_a_enviar.numero_cliente_follow_unfollow = usuario_a_dejar_de_seguir;

    do {
       id_pipe_cliente_a_servidor = open(pipe_cliente_a_servidor, O_WRONLY | O_NONBLOCK);
       if (id_pipe_cliente_a_servidor == -1) {
           perror("pipe");
           printf(" Se volvera a intentar despues\n");
           sleep(10);
       } else creado = 1;
    } while (creado == 0);

    write(id_pipe_cliente_a_servidor, &mensaje_a_enviar , sizeof(mensajeDelCliente));

    printf("Solicitud de unfollow enviada al servidor\n");
    confirmacionSenal = kill (pidServidor, SIGUSR1); /*enviar la señal*/
    if(confirmacionSenal == -1){
        perror("No se pudo enviar señal");
    }
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

void imprimirInstruccionesComando(){
    printf("El comando correcto es: ");
    printf("cliente -i <Id> -p <pipeNom>\n\n");
    printf("donde <Id> es el número del cliente\n");
    printf("y <pipeNom> es el nombre del pipe inicial para comunicación con el servidor\n");
    printf("y las banderas -i y -p son obligatorias\n\n");
}

int main (int argc, char **argv){

    int id_pipe_inicial, pid, creado = 0, res;
    comunicacionInicialCliente datosProcesoCliente;
    int opcion = 0;
    int numero_bytes;
    int status, c;
    char cOpcion;
    int contador=0;
    char stringPidProceso[10];
    char* pipeInicial;

    mode_t fifo_mode = S_IRUSR | S_IWUSR;

    clienteAceptado = false;

    /* Instalar manejador de la señal */
    signal (SIGUSR1, (sighandler_t)signalHandler);

    if(argc!=5){ /*No pueden haber más ni menos de 5 argumentos*/
        printf("\nError con número de argumentos\n\n");
        imprimirInstruccionesComando();
        exit(1);
    }else{
        if( !( (strcmp(argv[1],"-i") == 0 && strcmp(argv[3],"-p") == 0) ||
            (strcmp(argv[1],"-p") == 0 && strcmp(argv[3],"-i") == 0) ) ){

            printf("\nError con las banderas del comando\n\n");
            imprimirInstruccionesComando();
            exit(1);
        }
    }/*end if*/

    if( strcmp(argv[1],"-i") == 0 && strcmp(argv[3],"-p") ==0 ){
        idCliente = atoi(argv[2]);
        pipeInicial = argv[4];
    }else{
        idCliente = atoi(argv[4]);
        pipeInicial = argv[2];
    }

    /*printf("Pipe inicial: %s\n", pipeInicial);*/

    pid = getpid();

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
                printf("Error: por favor ingrese un número válido (entre 1 y 4)\n");
                printf("Digite numero de la opcion: ");
		            status = scanf("%c", &cOpcion);
                while ((c = fgetc(stdin)) != '\n' && c != EOF){
                    contador = contador + 1;
                }
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
