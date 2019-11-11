/*
Nombre del archivo: gestor.c
Autores: Adrián García y Luis Rosales
Objetivo: implementa las funciones que usa el proceso servidor. En términos
  generales provee implementaciones par iniciar el proceso servidor, enviar datos
  a los procesos clientes a través de pipes y recibir datos de los clientes también
  por medio de pipes a raíz del envío de señales.
Funciones: AdicionarSeguidor, RemoverSeguidor, Desconectar, GuardarTweet,
  EnviarTweetsASeguidorRecienConectado, EnviarTweetASeguidoresConectados,
  ManejadorSenalOperaciones, ManejarNuevaConexion, ImprimirInstruccionesComando,
  ImprimirInformacionEstructuraUsuarios, main, ManejadorSenalDesconexion
Fecha de última modificación: 04/11/19
*/

#include "gestor.h"

/*
Función: ManejadorSenalOperaciones
Autores de la función: Adrián García y Luis Rosales.
Parámetros de entrada: ninguno.
Retorno: ninguno.
Descripción: se ejecuta al recibirse una señal de tipo SIGUSR1, enviada por algún
cliente. Una vez recibida, se extraen los datos enviados por el cliente a través
del pipe cliente_a_servidor. Dependiendo de la operación de la que se recibió
solicitud (conexión, follow, unfollow, tweet o desconexión) por parte del cliente,
se hace el llamado a la función específica para manejar la solicitud. Durante la
ejecución, se imprime por pantalla la información más importante de las
operaciones efectuadas.
Variables globales usadas: numero_usuarios, clientes_estados
*/
sighandler_t ManejadorSenalOperaciones (void){

    int i, j, id_pipe_cliente_a_servidor, id_pipe_servidor_a_cliente, creado = 0;
    int numero_bytes;
    mensaje_del_cliente mensaje_recibido;
    int confirmacion_senal;

    int numero_usuario_seguidor, numero_usuario_a_seguir, numero_usuario_seguido;

    printf("\n\nRecibiendo un paquete de un cliente\n");

    for(i=0;i<numero_usuarios;i++){

        if ( clientes_estados[i].activo == true ){

            creado = 0;
            do {
               id_pipe_cliente_a_servidor = open(clientes_estados[i].pipe_cliente_a_servidor, O_RDONLY | O_NONBLOCK);
               if (id_pipe_cliente_a_servidor == -1) {
                  perror("Server abriendo el pipe especifico\n");
                  printf("Se volvera a intentar despues\n");
                  sleep(5);
               } else creado = 1;
            }  while (creado == 0);
            /*end do while*/

            numero_bytes = read (id_pipe_cliente_a_servidor, &mensaje_recibido, sizeof(mensaje_del_cliente) );
            if(numero_bytes > 0){
                printf("Recibido paquete del cliente con id %d y pid %d\n",  mensaje_recibido.numero_cliente,
                    mensaje_recibido.pid);

                switch (mensaje_recibido.operacion){
                    case 1:
                        printf("Cliente solicitó ejecutar follow\n");
                        numero_usuario_seguidor = mensaje_recibido.numero_cliente;
                        numero_usuario_a_seguir = mensaje_recibido.numero_cliente_follow_unfollow;
                        AdicionarSeguidor(numero_usuario_seguidor, numero_usuario_a_seguir);
                        break;
                    case 2:
                        printf("Cliente solicitó ejecutar unfollow\n");
                        numero_usuario_seguidor = mensaje_recibido.numero_cliente;
                        numero_usuario_seguido = mensaje_recibido.numero_cliente_follow_unfollow;
                        RemoverSeguidor(numero_usuario_seguidor, numero_usuario_seguido);
                        break;
                    case 3:
                        printf("Cliente envió un tweet\n");
                        printf("Tweet recibido: %s\n", mensaje_recibido.mensaje);
                        GuardarTweet(mensaje_recibido);
                        EnviarTweetASeguidoresConectados(mensaje_recibido);
                        break;
                }/*end switch*/

            }/*end if*/

        }/*end if*/

    }/*end for*/

    printf("El paquete ha sido procesado\n\n");
}

/*
Función: ManejadorSenalTerminacion
Autores de la función: Adrián García y Luis Rosales.
Parámetros de entrada: ninguno.
Retorno: ninguno.
Descripción: se ejecuta al recibirse la señal de terminación SIGINT (al cerrar
el servidor con CTRL+C). Se libera la memoria previamente asignada de
forma dinámica para almacenar toda la información de los usuarios,
presente en arreglo_usuarios. Además, se borra el pipe inicial creado para
recibir las nuevas conexiones de clientes.
Variables globales usadas: numero_usuarios, arreglo_usuarios, pipe_inicial
*/
sighandler_t ManejadorSenalTerminacion (void){
    int i,j;
    printf("Memoria liberada\n");
    printf("Se cierra servidor\n");
    char pipe_desconexion[TAMANO_NOMBRE_PIPE];

    strcpy(pipe_desconexion, pipe_inicial);
    strcat(pipe_desconexion, "_d");

    /*limpiar memoria*/
    for(i=0;i<numero_usuarios;i++){
        if(arreglo_usuarios[i].lista_siguiendo != NULL){
            free(arreglo_usuarios[i].lista_siguiendo);
            arreglo_usuarios[i].lista_siguiendo = NULL;
        }/*end if*/

        if(arreglo_usuarios[i].tweets != NULL){
            for(j=0;j<arreglo_usuarios[i].numero_tweets;j++){
                free(arreglo_usuarios[i].tweets[i]);
            }/*end for*/
            free(arreglo_usuarios[i].tweets);
            arreglo_usuarios[i].tweets = NULL;
        }/*end if*/
    }/*end for*/

    free(arreglo_usuarios);
    arreglo_usuarios = NULL;

    unlink(pipe_inicial);
    unlink(pipe_desconexion);

    exit(1);
}

/*
Función: ManejadorSenalDesconexion
Autores de la función: Adrián García y Luis Rosales.
Parámetros de entrada: ninguno.
Retorno: ninguno.
Descripción: se ejecuta al recibirse la señal de terminación SIGUSR2, enviada
por un cliente para avisar la desconexión. Se abre pipe_desconexion, se extrae
el paquete enviado por el cliente para reconocer su id y se llama a la función
Desconectar para actualizar el arreglo con la información de los usuarios
conectados.
Variables globales usadas: ninguna.
*/
sighandler_t ManejadorSenalDesconexion (void){

  int i, j, id_pipe, creado = 0;
  int numero_bytes;
  mensaje_del_cliente mensaje_recibido;
  char pipe_desconexion[TAMANO_NOMBRE_PIPE];

  strcpy(pipe_desconexion, pipe_inicial);
  strcat(pipe_desconexion, "_d");

  printf("\n\nRecibiendo un paquete de un cliente\n");

  creado = 0;
  do {
      id_pipe = open(pipe_desconexion, O_RDONLY | O_NONBLOCK);
      if (id_pipe == -1) {
          perror("Server abriendo el pipe especifico\n");
          printf("Se volvera a intentar despues\n");
          sleep(5);
      } else creado = 1;
  }  while (creado == 0);
  /*end do while*/

  numero_bytes = read (id_pipe, &mensaje_recibido, sizeof(mensaje_del_cliente) );
  if(numero_bytes > 0){
      printf("Recibido paquete del cliente con id %d y pid %d\n",  mensaje_recibido.numero_cliente,
          mensaje_recibido.pid);
      Desconectar(mensaje_recibido.numero_cliente-1);

      printf("Cliente con id %d y pid %d se ha desconectado\n",  mensaje_recibido.numero_cliente,
          mensaje_recibido.pid);
  }/*end if*/

  printf("El paquete ha sido procesado\n\n");
}

/*
Función: AdicionarSeguidor
Autores de la función: Adrián García y Luis Rosales.
Parámetros de entrada: un entero con el número del usuario que envió la
solicitud de follow y un entero con el número de usuario que se desea empezar
a seguir.
Retorno: ninguno.
Descripción: maneja la solicitud de follow enviada por un cliente. Si el usuario
al que se desea seguir no es válido o ya se sigue, entonces se responde al
cliente con un mensaje de error. En caso contrario, se actualiza el arreglo con
la información de los usuarios añadiendo el seguidor correspondiente y luego
se responde al cliente con un mensaje indicando que la solicitud fue exitosa.
Variables globales usadas: clientes_estados, arreglo_usuarios
*/
void AdicionarSeguidor(int numero_usuario_seguidor, int numero_usuario_a_seguir){

    int creado=0;
    int id_pipe_servidor_a_cliente, confirmacion_senal;
    mensaje_del_servidor mensaje_para_cliente;
    char string_usuario_a_seguir[TAMANO_STRING_ID_USUARIO];

    numero_usuario_seguidor = numero_usuario_seguidor-1; //para manejar correctamente índice
    numero_usuario_a_seguir = numero_usuario_a_seguir-1; //para manejar correctamente índice

    mensaje_para_cliente.pid = getpid();
    mensaje_para_cliente.operacion = 1;
    sprintf(string_usuario_a_seguir, "%d", numero_usuario_a_seguir+1);

    if(numero_usuario_a_seguir > numero_usuarios-1){ /*el usuario es mayor al número de usuarios del archivo, error*/
        strcpy(mensaje_para_cliente.mensaje, "Operación follow fallida: el usuario a seguir no existe");
        printf("Operación follow fallida: el usuario a seguir no existe\n");
    }else{
        if( arreglo_usuarios[numero_usuario_seguidor].lista_siguiendo[numero_usuario_a_seguir] == 0 ){
            /*printf("Antes: %d\n", arreglo_usuarios[numero_usuario_seguidor].lista_siguiendo[numero_usuario_a_seguir]);*/
            arreglo_usuarios[numero_usuario_seguidor].lista_siguiendo[numero_usuario_a_seguir] = 1;
            arreglo_usuarios[numero_usuario_seguidor].numero_siguiendo = arreglo_usuarios[numero_usuario_seguidor].numero_siguiendo+1;
            /*printf("despues: %d\n", arreglo_usuarios[numero_usuario_seguidor].lista_siguiendo[numero_usuario_a_seguir]);*/
            strcpy(mensaje_para_cliente.mensaje, "Operación follow exitosa: ahora sigues a usuario ");
            strcat(mensaje_para_cliente.mensaje, string_usuario_a_seguir);
            strcat(mensaje_para_cliente.mensaje, "\n");
            printf("Operación follow exitosa: usuario %d sigue ahora a usuario %d\n", numero_usuario_seguidor+1, numero_usuario_a_seguir+1);

        }else{ /*ya está siguiendo a ese usuario, enviar error*/

            strcpy(mensaje_para_cliente.mensaje, "Operación follow fallida: ya sigues a usuario ");
            strcat(mensaje_para_cliente.mensaje, string_usuario_a_seguir);
            strcat(mensaje_para_cliente.mensaje, "\n");
            printf("Operación follow fallida: usuario %d ya sigue a usuario %d\n", numero_usuario_seguidor+1, numero_usuario_a_seguir+1);
        }/*end if*/

    }/*end if*/

    do {
       id_pipe_servidor_a_cliente = open(clientes_estados[numero_usuario_seguidor].pipe_servidor_a_cliente, O_WRONLY | O_NONBLOCK);
       if (id_pipe_servidor_a_cliente == -1) {
          perror("Server abriendo el pipe especifico\n");
          printf("Se volvera a intentar despues\n");
          sleep(5);
       } else creado = 1;
    }  while (creado == 0);
    /*end do while*/

    write(id_pipe_servidor_a_cliente, &mensaje_para_cliente, sizeof(mensaje_del_servidor) );

    confirmacion_senal = kill (clientes_estados[numero_usuario_seguidor].pid, SIGUSR1); /*enviar la señal*/
    if(confirmacion_senal == -1){
        perror("No se pudo enviar señal");
    }/*end if*/
    printf("Enviada respuesta operación follow a cliente con id %d y pid %d\n",
        numero_usuario_seguidor+1, clientes_estados[numero_usuario_seguidor].pid);

}

/*
Función: RemoverSeguidor
Autores de la función: Adrián García y Luis Rosales.
Parámetros de entrada: n entero con el número del usuario que envió la
solicitud de unfollow y un entero con el número de usuario al que se desea dejar
de seguir.
Retorno: ninguno.
Descripción: maneja la solicitud de unfollow enviada por un cliente. Si el usuario
al que se desea dejar de seguir no es válido o no se sigue, entonces se responde
al cliente con un mensaje de error. En caso contrario, se actualiza el arreglo
con la información de los usuarios eliminando el seguidor correspondiente y luego
se responde al cliente con un mensaje indicando que la solicitud fue exitosa.
Variables globales usadas: clientes_estados, arreglo_usuarios
*/
void RemoverSeguidor(int numero_usuario_seguidor, int numero_usuario_seguido){

    int creado=0;
    int id_pipe_servidor_a_cliente, confirmacion_senal;
    mensaje_del_servidor mensaje_para_cliente;
    char string_usuario_seguido[TAMANO_STRING_ID_USUARIO];

    numero_usuario_seguidor = numero_usuario_seguidor-1; //para manejar correctamente índice
    numero_usuario_seguido = numero_usuario_seguido-1; //para manejar correctamente índice

    mensaje_para_cliente.pid = getpid();
    mensaje_para_cliente.operacion = 2;
    sprintf(string_usuario_seguido, "%d", numero_usuario_seguido+1);

    if(numero_usuario_seguido > numero_usuarios-1){ /*el usuario es mayor al número de usuarios del archivo, error*/
        strcpy(mensaje_para_cliente.mensaje, "Operación unfollow fallida: el usuario a dejar de seguir no existe");
        printf("Operación unfollow fallida: el usuario a dejar de seguir no existe\n");
    }else{
        if( arreglo_usuarios[numero_usuario_seguidor].lista_siguiendo[numero_usuario_seguido] == 1 ){

            arreglo_usuarios[numero_usuario_seguidor].lista_siguiendo[numero_usuario_seguido] = 0;
            arreglo_usuarios[numero_usuario_seguidor].numero_siguiendo = arreglo_usuarios[numero_usuario_seguidor].numero_siguiendo-1;
            strcpy(mensaje_para_cliente.mensaje, "Operación unfollow exitosa: ya no sigues a usuario ");
            strcat(mensaje_para_cliente.mensaje, string_usuario_seguido);
            strcat(mensaje_para_cliente.mensaje, "\n");
            printf("Operación unfollow exitosa: usuario %d ya no sigue a usuario %d\n", numero_usuario_seguidor+1, numero_usuario_seguido+1);

        }else{ /*no está siguiendo a ese usuario, enviar error*/
            strcpy(mensaje_para_cliente.mensaje, "Operación unfollow fallida: no sigues a usuario ");
            strcat(mensaje_para_cliente.mensaje, string_usuario_seguido);
            strcat(mensaje_para_cliente.mensaje, "\n");
            printf("Operación unfollow fallida: usuario %d no sigue a usuario %d\n", numero_usuario_seguidor+1, numero_usuario_seguido+1);
        }/*end if*/
    }/*end if*/

    do {
       id_pipe_servidor_a_cliente = open(clientes_estados[numero_usuario_seguidor].pipe_servidor_a_cliente, O_WRONLY | O_NONBLOCK);
       if (id_pipe_servidor_a_cliente == -1) {
          perror("Server abriendo el pipe especifico\n");
          printf("Se volvera a intentar despues\n");
          sleep(5);
       } else creado = 1;
    }  while (creado == 0);
    /*end do while*/

    write(id_pipe_servidor_a_cliente, &mensaje_para_cliente, sizeof(mensaje_del_servidor) );

    confirmacion_senal = kill (clientes_estados[numero_usuario_seguidor].pid, SIGUSR1); /*enviar la señal*/
    if(confirmacion_senal == -1){
        perror("No se pudo enviar señal");
    }/*end if*/
    printf("Enviada respuesta operación follow a cliente con id %d y pid %d\n",
        numero_usuario_seguidor+1, clientes_estados[numero_usuario_seguidor].pid);

}

/*
Función: Desconectar
Autores de la función: Adrián García y Luis Rosales.
Parámetros de entrada: un entero con el índice del usuario en el arreglo de
usuarios.
Retorno: ninguno.
Descripción: ejecuta la solicitud de desconexión realizada por un cliente. Se
actualiza el arreglo que guarda los estados de los usuarios (clientes_estados),
para indicar que dicho usuario ya no estará activo en el sistema.
Variables globales usadas: clientes_estados
*/
void Desconectar(int i){
    printf("Cliente solicitó desconexión\n");
    clientes_estados[i].activo = false;
    clientes_estados[i].pid = -1;
    strcpy(clientes_estados[i].pipe_cliente_a_servidor , "");
    strcpy(clientes_estados[i].pipe_servidor_a_cliente , "");
}

/*
Función: GuardarTweet
Autores de la función: Adrián García y Luis Rosales.
Parámetros de entrada: un objeto de tipo mensaje_del_cliente, que almacena el tweet
a guardar así como su información de procedencia.
Retorno: ninguno.
Descripción: guarda el tweet recibido en el arreglo con la información de los
usuarios. Al no conocerse a priori cuántos tweets va a hacer un usuario, estos
se almacenan de forma dinámica en un arreglo para cada usuario.
Variables globales usadas: arreglo_usuarios
*/
void GuardarTweet(mensaje_del_cliente mensaje_recibido){

    char** arreglo_tweets_aux;
    int id_tweetero, posicion_guardado;
    char* tweet_aux = (char*)malloc(sizeof(char)*TAMANO_TWEET);

    id_tweetero = mensaje_recibido.numero_cliente-1; /*para manejar los índices correctamente*/

    if( arreglo_usuarios[id_tweetero].numero_tweets == 0 ){
        arreglo_usuarios[id_tweetero].tweets = (char**)malloc(1*sizeof(char*));
            if(arreglo_usuarios[id_tweetero].tweets == NULL){
                perror("Memoria no alocada");
                exit(1);
            }/*end if*/
    }else{
        arreglo_tweets_aux = realloc(arreglo_usuarios[id_tweetero].tweets,
              (arreglo_usuarios[id_tweetero].numero_tweets+1) * sizeof(char*));
        if(arreglo_tweets_aux == NULL){
            perror("Memoria no alocada");
            exit(1);
        }/*end if*/
        arreglo_usuarios[id_tweetero].tweets = arreglo_tweets_aux;
        arreglo_tweets_aux = NULL;
    }/*end if*/
    posicion_guardado = arreglo_usuarios[id_tweetero].numero_tweets;

    strcpy(tweet_aux, mensaje_recibido.mensaje);

    arreglo_usuarios[id_tweetero].tweets[ posicion_guardado ] = tweet_aux;
    arreglo_usuarios[id_tweetero].numero_tweets = arreglo_usuarios[id_tweetero].numero_tweets+1;

    tweet_aux = NULL;

    printf("Tweet del usuario %d guardado\n", id_tweetero+1);
}

/*
Función: EnviarTweetsASeguidorRecienConectado
Autores de la función: Adrián García y Luis Rosales.
Parámetros de entrada: un objeto de tipo comunicacion_inicial_cliente, que
almacena la información del cliente que se acaba de solicitar conexión al
servidor.
Retorno: ninguno.
Descripción: envía a un cliente que acaba de conectarse, los tweets de los
usuarios a los que sigue, que se hayan realizado previamente a la conexión.
Revisa si el cliente que acaba de conectarse está siguiendo a otros usuarios, y
en caso de que si, entonces envía al cliente todos los tweets de los usuarios
de los que es seguidor.
Variables globales usadas: numero_usuarios, arreglo_usuarios
*/
void EnviarTweetsASeguidorRecienConectado(comunicacion_inicial_cliente nuevo_cliente){

    int id_pipe_servidor_a_cliente, confirmacion_senal, i, j, creado;
    mensaje_del_servidor mensaje_para_cliente;
    int indice = nuevo_cliente.numero_cliente-1;

    mensaje_para_cliente.pid = getpid();
    mensaje_para_cliente.operacion = 4;

    if(arreglo_usuarios[indice].numero_siguiendo > 0){ /*si está siguiendo a alguien*/
        for(i=0;i<numero_usuarios;i++){
            if( arreglo_usuarios[indice].lista_siguiendo[i] == 1 ){ /*si sigue a usuario i*/
                for(j=0;j<arreglo_usuarios[i].numero_tweets;j++){ /*la cantidad de tweets hechos por ese usuario i*/

                    creado = 0;
                    do {
                       id_pipe_servidor_a_cliente = open(nuevo_cliente.pipe_servidor_a_cliente, O_WRONLY | O_NONBLOCK);
                       if (id_pipe_servidor_a_cliente == -1) {
                          perror("Server abriendo el pipe especifico\n");
                          printf("Se volvera a intentar despues\n");
                          sleep(5);
                       } else creado = 1;
                    }  while (creado == 0);
                    /*end do while*/

                    /*copiar id de quien hizo el tweet, se hace +1 para manejo correcto de índice para mostrar en pantalla*/
                    mensaje_para_cliente.id_tweetero = i+1;

                    /*copiar mensaje del otro usuario para enviarlo*/
                    strcpy(mensaje_para_cliente.mensaje, arreglo_usuarios[i].tweets[j]);
                    printf("Tweet guardado para enviar: %s\n", mensaje_para_cliente.mensaje );

                    write(id_pipe_servidor_a_cliente, &mensaje_para_cliente, sizeof(mensaje_del_servidor) );
                    printf("Enviado tweet guardado a cliente con id %d y pid %d\n", indice+1, nuevo_cliente.pid);
                }/*end for*/
            }/*end if*/
        }/*end for*/
    }/*end if*/

}

/*
Función: EnviarTweetASeguidoresConectados
Autores de la función: Adrián García y Luis Rosales.
Parámetros de entrada: un objeto de tipo mensaje_del_cliente, que almacena el
tweet que se acaba de recibir junto con su información de procedencia.
Retorno: ninguno.
Descripción: envía un tweet que se acaba de recibir a todos los seguidores
conectados del cliente que escribió el tweet.
Variables globales usadas: numero_usuarios, clientes_estados, arreglo_usuarios
*/
void EnviarTweetASeguidoresConectados(mensaje_del_cliente mensaje_recibido){

  mensaje_del_servidor mensaje_para_cliente;
  int id_pipe_servidor_a_cliente, confirmacion_senal, i, j, creado;
  bool b;

  mensaje_para_cliente.pid = getpid();
  mensaje_para_cliente.operacion = 3;
  strcpy(mensaje_para_cliente.mensaje, mensaje_recibido.mensaje);
  mensaje_para_cliente.id_tweetero = mensaje_recibido.numero_cliente;

  for(i=0;i<numero_usuarios;i++){

      if ( clientes_estados[i].activo == true && arreglo_usuarios[i].numero_siguiendo > 0 &&
         arreglo_usuarios[i].lista_siguiendo[mensaje_recibido.numero_cliente-1] == 1 ){

            /*si está siguiendo a quien envió el tweet*/
              creado = 0;
              do {
                 id_pipe_servidor_a_cliente = open(clientes_estados[i].pipe_servidor_a_cliente, O_WRONLY | O_NONBLOCK);
                 if (id_pipe_servidor_a_cliente == -1) {
                    perror("Server abriendo el pipe especifico\n");
                    printf("Se volvera a intentar despues\n");
                    sleep(5);
                 } else creado = 1;
              }  while (creado == 0);
              /*end do while*/

              write(id_pipe_servidor_a_cliente, &mensaje_para_cliente, sizeof(mensaje_del_servidor) );

              confirmacion_senal = kill (clientes_estados[i].pid, SIGUSR1); /*enviar la señal*/
              if(confirmacion_senal == -1){
                  perror("No se pudo enviar señal");
              }/*end if*/
              printf("Enviado tweet a cliente con id %d y pid %d\n", i+1, clientes_estados[i].pid);
      }/*end if*/
  }/*end for*/

}

/*
Función: ManejarNuevaConexion
Autores de la función: Adrián García y Luis Rosales.
Parámetros de entrada: un objeto de tipo comunicacion_inicial_cliente, que
almacena la información del cliente que acaba de enviar una solicitud para
conectarse al servidor.
Retorno: ninguno
Descripción: revisa si el cliente que acaba de enviar una solicitud para
conectarse al servidor es válido dentro del sistema (existe en el archivo
relaciones y no está ya conectado). Si el cliente es válido para conectarse,
responde a éste con el número de operación 0. En caso contrario, responde al
cliente con número de operación -1 o -2 (dependiendo de la causa), para
indicar que la conexión no fue establecida.
Variables globales usadas: numero_usuarios, clientes_estados
*/
void ManejarNuevaConexion(comunicacion_inicial_cliente nuevo_cliente){

    int id_pipe_servidor_a_cliente, creado = 0, confirmacion_senal, id_pipe_cliente_a_servidor;
    mensaje_del_servidor mensaje_para_cliente;

    printf("Enviando respuesta inicial a cliente con id %d y pid %d\n", nuevo_cliente.numero_cliente, nuevo_cliente.pid);

    mensaje_para_cliente.pid = getpid();

    if (nuevo_cliente.numero_cliente>numero_usuarios){
      /*si ese cliente no tiene un id válido*/
        mensaje_para_cliente.operacion = -1;
    }else{
        if( clientes_estados[ nuevo_cliente.numero_cliente - 1 ].activo == true ){
          /*si ese cliente ya está conectado entonces esta nueva conexión no es válida*/
            mensaje_para_cliente.operacion = -2;
        }else{
            mensaje_para_cliente.operacion = 0;
            clientes_estados[ nuevo_cliente.numero_cliente - 1 ].pid = nuevo_cliente.pid;
            strcpy(clientes_estados[ nuevo_cliente.numero_cliente - 1 ].pipe_cliente_a_servidor , nuevo_cliente.pipe_cliente_a_servidor);
            strcpy(clientes_estados[ nuevo_cliente.numero_cliente - 1 ].pipe_servidor_a_cliente , nuevo_cliente.pipe_servidor_a_cliente);
            clientes_estados[ nuevo_cliente.numero_cliente - 1 ].activo = true;

            creado = 0;
            do {
               id_pipe_cliente_a_servidor = open(nuevo_cliente.pipe_cliente_a_servidor, O_RDONLY | O_NONBLOCK);
               if (id_pipe_cliente_a_servidor == -1) {
                  perror("\nServer abriendo el pipe especifico\n");
                  printf("Se volvera a intentar despues\n");
                  sleep(5);
               } else creado = 1;
            }  while (creado == 0);
            /*end do while*/
        }/*end if*/
    }/*end if*/

    do {
       id_pipe_servidor_a_cliente = open(nuevo_cliente.pipe_servidor_a_cliente, O_WRONLY| O_NONBLOCK);
       if ( id_pipe_servidor_a_cliente == -1 ) {
          perror("\nServer abriendo el pipe especifico\n");
          printf("Se volvera a intentar despues\n");
          sleep(5);
       } else creado = 1;
    }  while (creado == 0);
    /*end do while*/

    write(id_pipe_servidor_a_cliente, &mensaje_para_cliente, sizeof(mensaje_del_servidor) );
    printf("Enviada respuesta inicial a cliente con id %d y pid %d\n", nuevo_cliente.numero_cliente, nuevo_cliente.pid);

    if(mensaje_para_cliente.operacion == 0){ /*si la conexión fue exitosa*/
        EnviarTweetsASeguidorRecienConectado(nuevo_cliente);
    }/*end if*/

    confirmacion_senal = kill (nuevo_cliente.pid, SIGUSR1); /*enviar la señal*/
    if(confirmacion_senal == -1){
        perror("No se pudo enviar señal");
    }/*end if*/
    printf("\n");

}

/*
Función: ImprimirInstruccionesComando
Autores de la función: Luis Rosales.
Parámetros de entrada: ninguno.
Retorno: ninguno.
Descripción: imprime las instrucciones generales de cómo ejecutar correctamente
el comando para iniciar el proceso servidor.
Variables globales usadas: ninguna.
*/
void ImprimirInstruccionesComando(){
    printf("El comando correcto es: ");
    printf("gestor -r <relaciones> -p <pipeNom>\n\n");
    printf("donde <relaciones> es el archivo que contiene los datos de los clientes\n");
    printf("y <pipeNom> es el nombre del pipe inicial para comunicación con los clientes\n");
    printf("y las banderas -r y -p son obligatorias\n\n");
}

/*
Función: ImprimirInformacionEstructuraUsuarios
Autores de la función: Adrián García y Luis Rosales.
Parámetros de entrada: ninguno.
Retorno: ninguno.
Descripción: imprime la información de los usuarios que se encuentra
almacenada en el arreglo que guarda ésta: donde se indica el número del usuario,
cuántos usuarios sigue y a quiénes sigue.
Variables globales usadas: numero_usuarios, arreglo_usuarios
*/
void ImprimirInformacionEstructuraUsuarios(){

    int i,j;

    for(i=0;i<numero_usuarios;i++){

        printf("\n\nUsuario %d\n", i+1);
        printf("Siguiendo a %d usuarios",arreglo_usuarios[i].numero_siguiendo);
        if(arreglo_usuarios[i].numero_siguiendo != 0){
            printf("\nSiguiendo a:  ");
            for(j=0;j<numero_usuarios;j++){
                if( arreglo_usuarios[i].lista_siguiendo[j] == 1 ){
                    printf("%d ",j+1);
                }/*end if*/
            }/*end for*/
        }/*end if*/

    }/*end for*/
}

/*
Función: main
Autores de la función: Adrián García y Luis Rosales.
Parámetros de entrada: n entero con el número de argumentos recibidos por
consola y un arreglo de los argumentos recibidos.
Retorno: un entero indicando si el programa finaliza.
Descripción: es la función que inicia el proceso, recibiendo y validando
los argumentos introducidos por consola. Hace llamado a las funciones que leeen
el archivo de relaciones e inicializa los arreglos para guardar la información
de los usuarios. También inicializa el pipe por el que se hacen solicitudes de
conexión al servidor. Mantiene un ciclo infinito que  constantemente revisa si
un nuevo cliente realiza solicitud de conexión. En caso de solicitud de
conexión, llama a la función especializada en manejar ésta. Además, instala el
manejador de la señal para manejar las solicitudes diferentes a conexión.
Variables globales usadas: numero_usuarios, clientes_estados, arreglo_usuarios,
pipe_inicial
*/
int main (int argc, char **argv){

    int id_pipe_inicial, pid, n, cuantos,res,creado=0, i, j, id_pipe_desconexion;
    comunicacion_inicial_cliente datos_proceso_cliente;
    char nombre_archivo[TAMANO_NOMBRE_ARCHIVO];
    int numero_lineas_archivo = 0;
    int numero_siguiendo = 0;
    int numero_cliente_conectado;
    int numero_bytes;
    char pipe_desconexion[TAMANO_NOMBRE_PIPE];

    mode_t fifo_mode = S_IRUSR | S_IWUSR;

    if(argc!=5){ /*No pueden haber más ni menos de 5 argumentos*/
        printf("\nError con número de argumentos\n\n");
        ImprimirInstruccionesComando();
        exit(1);
    }else{
        if( !( (strcmp(argv[1],"-r") == 0 && strcmp(argv[3],"-p") == 0) ||
            (strcmp(argv[1],"-p") == 0 && strcmp(argv[3],"-r") == 0) ) ){

            printf("\nError con las banderas del comando\n\n");
            ImprimirInstruccionesComando();
            exit(1);
        }/*end if*/
    }/*end if*/

    if( strcmp(argv[1],"-r") == 0 && strcmp(argv[3],"-p") ==0 ){
        strcpy(nombre_archivo, argv[2]);
        pipe_inicial = argv[4];
    }else{
        strcpy(nombre_archivo, argv[4]);
        pipe_inicial = argv[2];
    }/*end if*/
    strcpy(pipe_desconexion, pipe_inicial);
    strcat(pipe_desconexion, "_d");

    signal (SIGINT, (sighandler_t)ManejadorSenalTerminacion); /* Instalar manejador de la señal */
    signal (SIGUSR1, (sighandler_t)ManejadorSenalOperaciones); /* Instalar manejador de la señal */
    signal (SIGUSR2, (sighandler_t)ManejadorSenalDesconexion); /* Instalar manejador de la señal */

    /* Creacion del pipe inicial, el que se recibe como argumento del main */
    unlink(pipe_inicial);
    if (mkfifo (pipe_inicial, fifo_mode) == -1) {
       perror("Server mkfifo");
       exit(1);
    }/*end if*/

    unlink(pipe_desconexion);
    if (mkfifo (pipe_desconexion, fifo_mode) == -1) {
       perror("Server mkfifo");
       exit(1);
    }/*end if*/

    do {
       id_pipe_desconexion = open(pipe_desconexion, O_RDONLY | O_NONBLOCK);
       if (id_pipe_desconexion == -1) {
           perror("pipe");
           printf(" Se volvera a intentar despues\n");
           sleep(10);
       } else creado = 1;
    } while (creado == 0);
    /*end do while*/

    /*leer el archivo e imprimir la información almacenada en este*/
    if( AbrirArchivo(nombre_archivo) == false ){
        exit(1);
    }/*end if*/

    numero_lineas_archivo = ContarLineasArchivo(nombre_archivo);

    /*llenar la estructura de usuarios con los datos del archivo*/
    arreglo_usuarios = LeerArchivo(nombre_archivo, numero_lineas_archivo);
    numero_usuarios = numero_lineas_archivo;

    printf("\n\nInformación obtenida a partir del archivo %s", nombre_archivo);

    /*imprimir la información obtenida a partir del archivo*/
    ImprimirInformacionEstructuraUsuarios();

    printf("\n\nFin de la información obtenida a partir del archivo %s\n\n", nombre_archivo);

    /*inicializar el arreglo para guardar la información de los clientes conectados*/
    clientes_estados = (estados_clientes*)malloc(numero_usuarios*sizeof(estados_clientes));
    if (clientes_estados == NULL) {
        perror("Memoria no alocada");
        exit(1);
    }/*end if*/

    for(i=0;i<numero_usuarios;i++){
        clientes_estados[i].pid = 0;
        clientes_estados[i].activo = false;
        strcpy(clientes_estados[i].pipe_cliente_a_servidor , "");
        strcpy(clientes_estados[i].pipe_servidor_a_cliente , "");
    }/*end for*/

    /*ejecutar hasta seleccionar desconexión*/
    while(1){

        creado = 0;
        do {
           id_pipe_inicial = open (pipe_inicial, O_RDONLY);
           if (id_pipe_inicial == -1) {
               perror("Pipe: ");
               printf(" Se volvera a intentar despues\n");
               sleep(5);
           } else creado = 1;
        } while (creado == 0);
        /*end do while*/

        numero_bytes = read (id_pipe_inicial, &datos_proceso_cliente, sizeof(comunicacion_inicial_cliente) );
        if (numero_bytes == -1){
            perror("proceso servidor: ");
            exit(1);
        }/*end if*/

        if(numero_bytes > 0){

            printf("\n\nSe conectó un cliente con id %d y pid %d\n", datos_proceso_cliente.numero_cliente
              ,datos_proceso_cliente.pid);
            printf("Comunicación con cliente con id %d y pid %d se hará a través de %s y %s\n",
              datos_proceso_cliente.numero_cliente,datos_proceso_cliente.pid,
              datos_proceso_cliente.pipe_cliente_a_servidor,
              datos_proceso_cliente.pipe_servidor_a_cliente);
            ManejarNuevaConexion(datos_proceso_cliente);

        }/*end if*/
    }/*end while*/

    exit(0);

}
