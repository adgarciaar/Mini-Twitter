/*
Nombre del archivo: cliente.c
Autores: Adrián García y Luis Rosales
Objetivo: implementa las funciones que usa cada proceso cliente. En términos
  generales provee implementaciones par iniciar el proceso cliente, enviar datos
  al proceso servidor a través de pipes y recibir datos del servidor también por
  medio de pipes a raíz del envío de una señal.
Funciones: ManejadorSenal, Follow, Unfollow, EnviarTweet, Desconectar,
  ImprimirInstruccionesComando, main
Fecha de última modificación: 04/11/19
*/

#include "cliente.h"

/*
Función: ManejadorSenal
Autores de la función: Adrián García y Luis Rosales.
Parámetros de entrada: ninguno.
Retorno: ninguno.
Descripción: se ejecuta al recibirse una señal de tipo SIGUSR1, enviada por el
servidor. Una vez recibida, se extraen los datos enviados por el servidor a través
del pipe servidor_a_cliente. Dependiendo de la operación de la que se recibió
respuesta (conexión, follow, unfollow, tweet o desconexión) del servidor, se
realiza una impresión específica por pantalla para comunicar los resultasdos
al cliente.
Variables globales usadas: id_pipe_servidor_a_cliente, pipe_servidor_a_cliente,
pid_servidor, pipe_cliente_a_servidor
*/
sighandler_t ManejadorSenal (void){

    int numero_bytes;
    int creado;
    mensaje_del_servidor mensaje_recibido;
    mensaje_del_servidor mensaje_con_tweet_anterior;

    do {
       id_pipe_servidor_a_cliente = open(pipe_servidor_a_cliente, O_RDONLY | O_NONBLOCK);
       if (id_pipe_servidor_a_cliente == -1) {
           perror("pipe");
           printf(" Se volverá a intentar después\n");
           sleep(10);
       } else creado = 1;
       /*end if*/
    } while (creado == 0);
    /*end do while*/

    numero_bytes = read (id_pipe_servidor_a_cliente, &mensaje_recibido, sizeof(mensaje_del_servidor) );
    if(numero_bytes == -1){
        perror("No se pudo leer un pipe: ");
        exit(1);
    }/*end if*/

    printf("\n\n--------------Mensaje recibido desde el servidor--------------\n\n");

    pid_servidor = mensaje_recibido.pid;

    switch (mensaje_recibido.operacion){
      case -2:
          printf("El id del cliente no es válido, ya se encuentra conectado. Se procede a Desconectar.");
          break;
      case -1:
          printf("El id del cliente no es válido, no existe dentro del registro. Se procede a Desconectar.");
          break;
      case 0:
          printf("El id del cliente es válido");
          break;
      case 1: /*follow*/
          printf("Respuesta de operación follow:\n %s", mensaje_recibido.mensaje);
          break;
      case 2: /*Unfollow*/
          printf("Respuesta de operación unfollow:\n %s", mensaje_recibido.mensaje);
          break;
      case 3: /*tweet*/
          printf("Tweet realizado por cliente %d\n",mensaje_recibido.id_tweetero);
          printf("Tweet: %s\n", mensaje_recibido.mensaje);
          break;
    }/*end switch*/

    if(mensaje_recibido.operacion == 0){ /*se pueden recibir tweets hechos ya por otros usuarios*/

        do {
           id_pipe_servidor_a_cliente = open(pipe_servidor_a_cliente, O_RDONLY | O_NONBLOCK);
           if (id_pipe_servidor_a_cliente == -1) {
               perror("pipe");
               printf(" Se volverá a intentar después\n");
               sleep(10);
           } else creado = 1;
           /*end if*/
        } while (creado == 0);
        /*end do while*/

        numero_bytes = read (id_pipe_servidor_a_cliente, &mensaje_con_tweet_anterior, sizeof(mensaje_del_servidor) );
        if(numero_bytes == -1){
            perror("No se pudo leer un pipe: ");
            exit(1);
        }/*end if*/
        while(numero_bytes>0){
            if(mensaje_con_tweet_anterior.operacion == 4){
                printf("\nMientras no estabas\n");
                printf("Tweet realizado por cliente %d\n",mensaje_con_tweet_anterior.id_tweetero);
                printf("Tweet: %s\n", mensaje_con_tweet_anterior.mensaje);
            }
            numero_bytes = read (id_pipe_servidor_a_cliente, &mensaje_con_tweet_anterior, sizeof(mensaje_del_servidor) );
            if(numero_bytes == -1){
                perror("No se pudo leer un pipe: ");
                exit(1);
            }/*end if*/
        }/*end while*/
    }/*end if*/

    printf("\n\n--------------Mensaje recibido desde el servidor--------------\n\n");
    if(mensaje_recibido.operacion == -1 || mensaje_recibido.operacion == -2){
        unlink(pipe_cliente_a_servidor);
        unlink(pipe_servidor_a_cliente);
        printf("Eliminado: %s\n", pipe_cliente_a_servidor);
        printf("Eliminado: %s\n", pipe_servidor_a_cliente);
        printf("Desconexion realizada\n");
        exit(1);
    }else{
        cliente_aceptado = true;
    }/*end if*/

}

/*
Función: follow
Autores de la función: Adrián García y Luis Rosales.
Parámetros de entrada: ninguno.
Retorno: ninguno.
Descripción: solicita al usuario el número del usuario al que quiere seguir,
luego envía la solicitud de follow al servidor.
Variables globales usadas: id_pipe_cliente_a_servidor, pipe_cliente_a_servidor
*/
void Follow(){

    int confirmacion_senal, resultado_close;
    mensaje_del_cliente mensaje_a_enviar;
    int creado = 0, status, c, contador=0, resultado_write;
    int usuario_a_seguir;

    printf("Por favor ingrese el número del usuario a seguir: ");

    status = scanf("%d", &usuario_a_seguir);
    while ((c = fgetc(stdin)) != '\n' && c != EOF){ /* Flush stdin */
        contador = contador + 1;
    }; /*end while*/

    while(contador>0 || status!=1 || usuario_a_seguir <= 0 || usuario_a_seguir == id_cliente){
        contador = 0;
        if( usuario_a_seguir <= 0 ){
            printf("Error: el número debe ser mayor a cero\n");
        }/*end if*/
        if( usuario_a_seguir == id_cliente ){
            printf("Error: no te puedes seguir a ti mismo\n");
        }/*end if*/
        if (contador>0 || status!=1){
          printf("Error: por favor ingrese un número\n");
        }/*end if*/
        printf("Por favor ingrese el número del usuario a seguir: ");
        status = scanf("%d", &usuario_a_seguir);
        while ((c = fgetc(stdin)) != '\n' && c != EOF){
            contador = contador + 1;
        }/*end while*/
    }/*end while*/

    mensaje_a_enviar.pid = getpid();
    mensaje_a_enviar.operacion = 1;
    mensaje_a_enviar.numero_cliente = id_cliente;
    mensaje_a_enviar.numero_cliente_follow_unfollow = usuario_a_seguir;

    do {
       id_pipe_cliente_a_servidor = open(pipe_cliente_a_servidor, O_WRONLY | O_NONBLOCK);
       if (id_pipe_cliente_a_servidor == -1) {
           perror("pipe");
           printf(" Se volvera a intentar despues\n");
           sleep(10);
       } else creado = 1;
       /*end if*/
    } while (creado == 0);
    /*end do while*/

    resultado_write = write(id_pipe_cliente_a_servidor, &mensaje_a_enviar , sizeof(mensaje_del_cliente));
    if(resultado_write ==-1){
        perror("No se pudo escribir en un pipe: ");
        exit(1);
    }
    resultado_close = close(id_pipe_cliente_a_servidor);
    if(resultado_close == -1){
        perror("No se pudo cerrar un pipe del lado escritor: ");
        exit(1);
    }

    printf("Solicitud de follow enviada al servidor\n");
    confirmacion_senal = kill (pid_servidor, SIGUSR1); /*enviar la señal*/
    if(confirmacion_senal == -1){
        perror("No se pudo enviar señal");
        exit(1);
    }/*end if*/

}

/*
Función: Unfollow
Autores de la función: Adrián García y Luis Rosales.
Parámetros de entrada: ninguno.
Retorno: ninguno.
Descripción: solicita al usuario el número del usuario al que quiere dejar de
seguir, luego envía al servidor la solicitud de Unfollow.
Variables globales usadas: id_pipe_cliente_a_servidor, pipe_cliente_a_servidor
*/
void Unfollow(){

    int confirmacion_senal, resultado_write, resultado_close;
    mensaje_del_cliente mensaje_a_enviar;
    int creado = 0, status, c, contador=0;
    int usuario_a_dejar_de_seguir;

    printf("Por favor ingrese el número del usuario a dejar de seguir: ");

    status = scanf("%d", &usuario_a_dejar_de_seguir);
    while ((c = fgetc(stdin)) != '\n' && c != EOF){ /* Flush stdin */
        contador = contador + 1;
    }; /*end while*/

    while(contador>0 || status!=1 || usuario_a_dejar_de_seguir <= 0 || usuario_a_dejar_de_seguir == id_cliente){
        contador = 0;
        if( usuario_a_dejar_de_seguir <= 0 ){
            printf("Error: el número debe ser mayor a cero\n");
        }/*end if*/
        if( usuario_a_dejar_de_seguir == id_cliente ){
            printf("Error: no te puedes dejar de seguir a ti mismo (porque no te puedes seguir a ti mismo)\n");
        }/*end if*/
        if (contador>0 || status!=1){
          printf("Error: por favor ingrese un número\n");
        }/*end if*/
        printf("Por favor ingrese el número del usuario a dejar de seguir: ");
        status = scanf("%d", &usuario_a_dejar_de_seguir);
        while ((c = fgetc(stdin)) != '\n' && c != EOF){
            contador = contador + 1;
        }/*end while*/
    }/*end while*/

    mensaje_a_enviar.pid = getpid();
    mensaje_a_enviar.operacion = 2;
    mensaje_a_enviar.numero_cliente = id_cliente;
    mensaje_a_enviar.numero_cliente_follow_unfollow = usuario_a_dejar_de_seguir;

    do {
       id_pipe_cliente_a_servidor = open(pipe_cliente_a_servidor, O_WRONLY | O_NONBLOCK);
       if (id_pipe_cliente_a_servidor == -1) {
           perror("pipe");
           printf(" Se volvera a intentar despues\n");
           sleep(10);
       } else creado = 1;
       /*end if*/
    } while (creado == 0);
    /*end do while*/

    resultado_write = write(id_pipe_cliente_a_servidor, &mensaje_a_enviar , sizeof(mensaje_del_cliente));
    if(resultado_write ==-1){
        perror("No se pudo escribir en un pipe: ");
        exit(1);
    }
    resultado_close = close(id_pipe_cliente_a_servidor);
    if(resultado_close == -1){
        perror("No se pudo cerrar un pipe del lado escritor: ");
        exit(1);
    }

    printf("Solicitud de unfollow enviada al servidor\n");
    confirmacion_senal = kill (pid_servidor, SIGUSR1); /*enviar la señal*/
    if(confirmacion_senal == -1){
        perror("No se pudo enviar señal");
        exit(1);
    }/*end if*/
}

/*
Función: EnviarTweet
Autores de la función: Adrián García y Luis Rosales.
Parámetros de entrada: ninguno.
Retorno: ninguno.
Descripción: solicita al usuario introducir el tweet que quiere enviar y luego
envía este al servidor.
Variables globales usadas: id_pipe_cliente_a_servidor, pipe_cliente_a_servidor
*/
void EnviarTweet(){

    int confirmacion_senal, resultado_write, resultado_close;
    mensaje_del_cliente mensaje_a_enviar;
    char tweet[TAMANO_TWEET];
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
    }/*end while*/

    if(contador > 0){
        printf("Error: el tweet supera los 200 caracteres\n");
    }else{
        mensaje_a_enviar.pid = getpid();
        mensaje_a_enviar.operacion = 3;
        mensaje_a_enviar.numero_cliente = id_cliente;
        strcpy(mensaje_a_enviar.mensaje , tweet);

        do {
           id_pipe_cliente_a_servidor = open(pipe_cliente_a_servidor, O_WRONLY | O_NONBLOCK);
           if (id_pipe_cliente_a_servidor == -1) {
               perror("pipe");
               printf(" Se volvera a intentar despues\n");
               sleep(10);
           } else creado = 1;
           /*end if*/
        } while (creado == 0);
        /*end do while*/

        resultado_write = write(id_pipe_cliente_a_servidor, &mensaje_a_enviar , sizeof(mensaje_del_cliente));
        if(resultado_write ==-1){
            perror("No se pudo escribir en un pipe: ");
            exit(1);
        }
        resultado_close = close(id_pipe_cliente_a_servidor);
        if(resultado_close == -1){
            perror("No se pudo cerrar un pipe del lado escritor: ");
            exit(1);
        }

        printf("Tweet enviado al servidor\n");
        confirmacion_senal = kill (pid_servidor, SIGUSR1); /*enviar la señal*/
        if(confirmacion_senal == -1){
            perror("No se pudo enviar señal");
            exit(1);
        }/*end if*/
    }/*end if*/
}

/*
Función: Desconectar
Autores de la función: Adrián García y Luis Rosales.
Parámetros de entrada: ninguno.
Retorno: ninguno.
Descripción: envía al servidor una solicitud de desconexión.
Variables globales usadas: ninguna.
*/
void Desconectar(char* pipe_inicial){

    int confirmacion_senal, resultado_write, resultado_close;
    int creado = 0;
    mensaje_del_cliente mensaje_a_enviar;
    int id_pipe;
    char pipe_desconexion[TAMANO_NOMBRE_PIPE];

    strcpy(pipe_desconexion, pipe_inicial);
    strcat(pipe_desconexion, "_d");

    mensaje_a_enviar.pid = getpid();
    mensaje_a_enviar.operacion = 4;
    mensaje_a_enviar.numero_cliente = id_cliente;

    do {
       id_pipe = open(pipe_desconexion, O_WRONLY | O_NONBLOCK);
       if (id_pipe == -1) {
           perror("pipe");
           printf(" Se volvera a intentar despues\n");
           sleep(10);
       } else creado = 1;
       /*end if*/
    } while (creado == 0);
    /*end do while*/

    resultado_write = write(id_pipe, &mensaje_a_enviar , sizeof(mensaje_del_cliente));
    if(resultado_write ==-1){
        perror("No se pudo escribir en un pipe: ");
        exit(1);
    }
    resultado_close = close(id_pipe);
    if(resultado_close == -1){
        perror("No se pudo cerrar un pipe del lado escritor: ");
        exit(1);
    }

    printf("Desconexion avisada al servidor\n");
    confirmacion_senal = kill (pid_servidor, SIGUSR2); /*enviar la señal*/
    if(confirmacion_senal == -1){
        perror("No se pudo enviar señal");
        exit(1);
    }/*end if*/
}

/*
Función: ImprimirInstruccionesComando
Autores de la función: Luis Rosales.
Parámetros de entrada: ninguno.
Retorno: ninguno.
Descripción: imprime las instrucciones generales de cómo ejecutar correctamente
el comando para iniciar el proceso cliente.
Variables globales usadas: ninguna
*/
void ImprimirInstruccionesComando(){
    printf("El comando correcto es: ");
    printf("cliente -i <Id> -p <pipeNom>\n\n");
    printf("donde <Id> es el número del cliente (mayor a cero)\n");
    printf("y <pipeNom> es el nombre del pipe inicial para comunicación con el servidor\n");
    printf("y las banderas -i y -p son obligatorias\n\n");
}

/*
Función: main
Autores de la función: Adrián García y Luis Rosales.
Parámetros de entrada: un entero con el número de argumentos recibidos por
consola y un arreglo de los argumentos recibidos.
Retorno: un entero indicando si el programa finaliza.
Descripción: es la función que inicia el proceso, recibiendo y validando
los argumentos introducidos por consola. Inicializa los pipes con los que el
cliente se va a comunicar con el servidor (uno de cliente a servidor y otro
de servidor a cliente). Mantiene un ciclo que muestra al usuario sus opciones
como cliente, recibiendo en cada iteración el número de la operación que el
usuario desea realizar. Este ciclo se realiza mientras el usuario no seleccione
la opción de desconexión. Para la operación solicitada, se llama a la función
especializada en realizarla. Además, instala el manejador de la señal para
recibir respuestas o mensajes enviados por el servidor.
Variables globales usadas: pipe_cliente_a_servidor, pipe_servidor_a_cliente,
id_pipe_servidor_a_cliente
*/
int main (int argc, char **argv){

    int id_pipe_inicial, pid, creado = 0, res, resultado_write;
    comunicacion_inicial_cliente datos_proceso_cliente;
    int opcion = 0;
    int numero_bytes, resultado_close, resultado_unlink;
    int status, c;
    char c_opcion;
    int contador=0;
    char string_pid_proceso[TAMANO_STRING_ID_PROCESO];
    char* pipe_inicial;

    mode_t fifo_mode = S_IRUSR | S_IWUSR;

    cliente_aceptado = false;

    /* Instalar manejador de la señal */
    signal (SIGUSR1, (sighandler_t)ManejadorSenal);

    if(argc!=5){ /*No pueden haber más ni menos de 5 argumentos*/
        printf("\nError con número de argumentos\n\n");
        ImprimirInstruccionesComando();
        exit(1);
    }else{
        if( !( (strcmp(argv[1],"-i") == 0 && strcmp(argv[3],"-p") == 0) ||
            (strcmp(argv[1],"-p") == 0 && strcmp(argv[3],"-i") == 0) ) ){

            printf("\nError con las banderas del comando\n\n");
            ImprimirInstruccionesComando();
            exit(1);
        }/*end if*/
    }/*end if*/

    if( strcmp(argv[1],"-i") == 0 && strcmp(argv[3],"-p") ==0 ){
        id_cliente = atoi(argv[2]);
        pipe_inicial = argv[4];
    }else{
        id_cliente = atoi(argv[4]);
        pipe_inicial = argv[2];
    }/*end if*/

    if(id_cliente<=0){
        printf("\nError: el id del cliente debe ser mayor a cero\n\n");
        ImprimirInstruccionesComando();
        exit(1);
    }/*end if*/

    pid = getpid();

    sprintf(string_pid_proceso, "%d", pid);

    strcat(pipe_cliente_a_servidor, string_pid_proceso);
    strcat(pipe_cliente_a_servidor, "_c_a_s");
    strcat(pipe_servidor_a_cliente, string_pid_proceso);
    strcat(pipe_servidor_a_cliente, "_s_a_c");

    datos_proceso_cliente.pid = pid;
    strcpy(datos_proceso_cliente.pipe_cliente_a_servidor, pipe_cliente_a_servidor);
    strcpy(datos_proceso_cliente.pipe_servidor_a_cliente, pipe_servidor_a_cliente);
    datos_proceso_cliente.numero_cliente = id_cliente;

    /*Se crea un pipe específico para la comunicación con el server.*/
    unlink(pipe_cliente_a_servidor);
    if (mkfifo (pipe_cliente_a_servidor, fifo_mode) == -1) {
       perror("Client  mkfifo");
       exit(1);
    }/*end if*/
    printf("\nPipe especifico creado: %s\n", pipe_cliente_a_servidor);

    /*Se crea un pipe específico para la comunicación con el server.*/
    unlink(pipe_servidor_a_cliente);
    if (mkfifo (pipe_servidor_a_cliente, fifo_mode) == -1) {
       perror("Client  mkfifo");
       exit(1);
    }/*end if*/
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
        /*end if*/
     } while (creado == 0);
     /*end do while*/

     /* Se abre el pipe cuyo nombre se recibe como argumento del main. */
     do {
        id_pipe_inicial = open(pipe_inicial, O_WRONLY | O_NONBLOCK);
        if (id_pipe_inicial == -1) {
            perror("pipe");
            printf(" Se volvera a intentar despues\n");
   	       sleep(10);
        } else creado = 1;
        /*end if*/
     } while (creado == 0);
     /*end do while*/

    /* se envia la información del cliente al servidor */
    resultado_write = write(id_pipe_inicial, &datos_proceso_cliente , sizeof(comunicacion_inicial_cliente));
    if(resultado_write ==-1){
        perror("No se pudo escribir en un pipe: ");
        exit(1);
    }
    resultado_close = close(id_pipe_inicial);
    if(resultado_close == -1){
        perror("No se pudo cerrar un pipe del lado escritor: ");
        exit(1);
    }

    do{
        if ( cliente_aceptado == true ){

            printf("\n\nOPCIONES\n\n");
            printf("Presione 1 para follow\n");
            printf("Presione 2 para unfollow\n");
            printf("Presione 3 para tweet\n");
            printf("Presione 4 para desconexion\n");
            printf("Digite numero de la opcion: ");

            status = scanf("%c", &c_opcion);
            while ((c = fgetc(stdin)) != '\n' && c != EOF){ /* Flush stdin */
                contador = contador + 1;
            }; /*end while*/

            while(contador>0 || (int)c_opcion <49 || (int)c_opcion>52){
                contador = 0;
                printf("Error: por favor ingrese un número válido (entre 1 y 4)\n");
                printf("Digite numero de la opcion: ");
		            status = scanf("%c", &c_opcion);
                while ((c = fgetc(stdin)) != '\n' && c != EOF){
                    contador = contador + 1;
                }/*end while*/
            }/*end while*/

            switch ( (int)c_opcion ){
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
            }/*end switch*/

            if(opcion >= 1 && opcion <=4){

                switch (opcion){
                    case 1:
                        Follow();
                        break;
                    case 2:
                        Unfollow();
                        break;
                    case 3:
                        EnviarTweet();
                        break;
                }/*end switch*/
            }/*end if*/
        }/*end if*/

    } while (opcion != 4);
    /*end do while*/

    Desconectar(pipe_inicial);
    resultado_unlink = unlink(pipe_cliente_a_servidor);
    if(resultado_unlink == -1){
        perror("No se pudo eliminar un pipe: ");
        /*No se hace exit aquí ya que se hará de todas formas enseguida*/
    }
    printf("Eliminado: %s\n", pipe_cliente_a_servidor);
    resultado_unlink = unlink(pipe_servidor_a_cliente);
    if(resultado_unlink == -1){
        perror("No se pudo eliminar un pipe: ");
        /*No se hace exit aquí ya que se hará de todas formas enseguida*/
    }
    printf("Eliminado: %s\n", pipe_servidor_a_cliente);
    printf("Desconexion realizada\n");
    exit(0);
}
