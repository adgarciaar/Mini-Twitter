/*
Autor:
Funcion:
Nota:
*/

#include "gestor.h"

void adicionarSeguidor(int numero_usuario_seguidor, int numero_usuario_a_seguir){

    int* arreglo_auxiliar = NULL;

    numero_usuario_seguidor = numero_usuario_seguidor-1; //para manejar correctamente índice
    numero_usuario_a_seguir = numero_usuario_a_seguir-1; //para manejar correctamente índice

    if( arreglo_usuarios[numero_usuario_seguidor].lista_siguiendo[numero_usuario_a_seguir] == 0 ){
        arreglo_usuarios[numero_usuario_seguidor].lista_siguiendo[numero_usuario_a_seguir] == 1;
        arreglo_usuarios[numero_usuario_seguidor].numero_siguiendo = arreglo_usuarios[numero_usuario_seguidor].numero_siguiendo+1;
    }else{ /*ya está siguiendo a ese usuario*/

    }

}

void removerSeguidor(int numero_usuario_seguidor, int numero_usuario_seguido){

    int i;
    numero_usuario_seguidor = numero_usuario_seguidor-1; //para manejar correctamente índice
    numero_usuario_seguido = numero_usuario_seguido-1; //para manejar correctamente índice

    if( arreglo_usuarios[numero_usuario_seguidor].lista_siguiendo[numero_usuario_seguido] == 1 ){
        arreglo_usuarios[numero_usuario_seguidor].lista_siguiendo[numero_usuario_seguido] == 0;
        arreglo_usuarios[numero_usuario_seguidor].numero_siguiendo = arreglo_usuarios[numero_usuario_seguidor].numero_siguiendo-1;
    }else{ /*no está siguiendo a ese usuario*/

    }

}

void desconectar(int i){
    printf("Cliente solicitó desconexión\n");
    clientesEstados[i].activo = false;
    clientesEstados[i].pid = -1;
    strcpy(clientesEstados[i].pipe_cliente_a_servidor , "");
    strcpy(clientesEstados[i].pipe_servidor_a_cliente , "");
}

void enviarTweet(mensajeDelCliente mensajeRecibido){

  mensajeDelServidor mensajeParaCliente;
  int id_pipe_servidor_a_cliente, confirmacionSenal, i, j, creado;
  bool b;

  mensajeParaCliente.pid = getpid();
  mensajeParaCliente.numeroMensajes = 1;
  strcpy(mensajeParaCliente.mensaje, mensajeRecibido.mensaje);
  mensajeParaCliente.idTweetero = mensajeRecibido.numeroCliente;

  for(i=0;i<numero_usuarios;i++){
      if ( clientesEstados[i].activo == true && arreglo_usuarios[i].numero_siguiendo > 0 &&
         arreglo_usuarios[i].lista_siguiendo[mensajeParaCliente.idTweetero-1] == 1 ){
            /*si está siguiendo a quien envió el tweet*/
              creado = 0;
              do {
                 id_pipe_servidor_a_cliente = open(clientesEstados[i].pipe_servidor_a_cliente, O_WRONLY | O_NONBLOCK);
                 if (id_pipe_servidor_a_cliente == -1) {
                    perror("Server abriendo el pipe especifico\n");
                    printf("Se volvera a intentar despues\n");
                    sleep(5);
                 } else creado = 1;
              }  while (creado == 0);

              write(id_pipe_servidor_a_cliente, &mensajeParaCliente, sizeof(mensajeDelServidor) );

              confirmacionSenal = kill (clientesEstados[i].pid, SIGUSR1); /*enviar la señal*/
              if(confirmacionSenal == -1){
                  perror("No se pudo enviar señal");
              }
              printf("Enviado tweet a cliente con id %d y pid %d\n", i+1, clientesEstados[i].pid);

      }
  }


}

sighandler_t signalHandler (void){

    int i, j, id_pipe_cliente_a_servidor, id_pipe_servidor_a_cliente, creado = 0;
    int numero_bytes;
    mensajeDelCliente mensajeRecibido;
    int confirmacionSenal;

    int numero_usuario_seguidor, numero_usuario_a_seguir, numero_usuario_seguido;

    printf("\n\nRecibiendo un paquete de un cliente\n");

    for(i=0;i<numero_usuarios;i++){

        if ( clientesEstados[i].activo == true ){

            creado = 0;
            do {
               id_pipe_cliente_a_servidor = open(clientesEstados[i].pipe_cliente_a_servidor, O_RDONLY | O_NONBLOCK);
               if (id_pipe_cliente_a_servidor == -1) {
                  perror("Server abriendo el pipe especifico\n");
                  printf("Se volvera a intentar despues\n");
                  sleep(5);
               } else creado = 1;
            }  while (creado == 0);

            numero_bytes = read (id_pipe_cliente_a_servidor, &mensajeRecibido, sizeof(mensajeDelCliente) );
            /*if (numero_bytes == -1){
                perror("proceso servidor: ");
                /*exit(1);
            }*/
            if(numero_bytes > 0){
                printf("Recibido paquete del cliente con id %d y pid %d\n",  mensajeRecibido.numeroCliente,
                    mensajeRecibido.pid);

                switch (mensajeRecibido.operacion){
                    case 1:
                        printf("Cliente solicitó ejecutar follow\n");
                        numero_usuario_seguidor = mensajeRecibido.numeroCliente;
                        numero_usuario_a_seguir = mensajeRecibido.numero_cliente_follow_unfollow;
                        adicionarSeguidor(numero_usuario_seguidor, numero_usuario_a_seguir);
                        break;
                    case 2:
                        printf("Cliente solicitó ejecutar unfollow\n");
                        numero_usuario_seguidor = mensajeRecibido.numeroCliente;
                        numero_usuario_seguido = mensajeRecibido.numero_cliente_follow_unfollow;
                        removerSeguidor(numero_usuario_seguidor, numero_usuario_seguido);
                        break;
                    case 3:
                        printf("Cliente envió un tweet\n");
                        printf("Tweet recibido: %s\n", mensajeRecibido.mensaje);
                        enviarTweet(mensajeRecibido);
                        break;
                    case 4: // se va a desconectar
                        desconectar(i);
                        printf("Se desconecta el cliente con id %d y pid %d\n",  mensajeRecibido.numeroCliente,
                            mensajeRecibido.pid);
                        break;
                }

            }

        }

    }

    printf("El paquete ha sido procesado\n\n");
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

    confirmacionSenal = kill (nuevoCliente.pid, SIGUSR1); /*enviar la señal*/
    if(confirmacionSenal == -1){
        perror("No se pudo enviar señal");
    }

    printf("Enviada respuesta inicial a cliente con id %d y pid %d\n\n", nuevoCliente.numeroCliente, nuevoCliente.pid);
}

void imprimirInstruccionesComando(){
    printf("El comando correcto es: ");
    printf("gestor -r <relaciones> -p <pipeNom>\n\n");
    printf("donde <relaciones> es el archivo que contiene los datos de los clientes\n");
    printf("y <pipeNom> es el nombre del pipe inicial para comunicación con los clientes\n");
    printf("y las banderas -r y -p son obligatorias\n\n");
}

int main (int argc, char **argv){

    int id_pipe_inicial, pid, n, cuantos,res,creado=0, i, j;
    comunicacionInicialCliente datosProcesoCliente;
    char nombre_archivo[30];
    char* pipeInicial;
    int numero_lineas_archivo = 0;
    int numero_siguiendo = 0;
    int numeroClienteConectado;
    int numero_bytes;

    mode_t fifo_mode = S_IRUSR | S_IWUSR;

    /*numero_usuarios = 10;*/

    if(argc!=5){ /*No pueden haber más ni menos de 5 argumentos*/
        printf("\nError con número de argumentos\n\n");
        imprimirInstruccionesComando();
        exit(1);
    }else{
        if( !( (strcmp(argv[1],"-r") == 0 && strcmp(argv[3],"-p") == 0) ||
            (strcmp(argv[1],"-p") == 0 && strcmp(argv[3],"-r") == 0) ) ){

            printf("\nError con las banderas del comando\n\n");
            imprimirInstruccionesComando();
            exit(1);
        }
    }/*end if*/

    if( strcmp(argv[1],"-r") == 0 && strcmp(argv[3],"-p") ==0 ){
        strcpy(nombre_archivo, argv[2]);
        pipeInicial = argv[4];
    }else{
        strcpy(nombre_archivo, argv[4]);
        pipeInicial = argv[2];
    }

    printf("Nombre archivo: %s\n", nombre_archivo);
    printf("Nombre pipe: %s\n", pipeInicial);

    signal (SIGUSR1, (sighandler_t)signalHandler); /* Instalar manejador de la señal */

    /* Creacion del pipe inicial, el que se recibe como argumento del main */
    unlink(pipeInicial);
    if (mkfifo (pipeInicial, fifo_mode) == -1) {
       perror("Server mkfifo");
       exit(1);
    }

    /*leer el archivo e imprimir la información almacenada en este*/
    if( AbrirArchivo(nombre_archivo) == false ){
        exit(1);
    }

    numero_lineas_archivo = ContarLineasArchivo(nombre_archivo);

    /*llenar la estructura de usuarios con los datos del archivo*/
    arreglo_usuarios = LeerArchivo(nombre_archivo, numero_lineas_archivo);
    numero_usuarios = numero_lineas_archivo;

    printf("\n\nInformación obtenida a partir del archivo %s", nombre_archivo);

    /*imprimir la información obtenida a partir del archivo*/
    for(i=0;i<numero_usuarios;i++){

        printf("\n\nUsuario %d\n", i+1);
        printf("Siguiendo a %d usuarios",arreglo_usuarios[i].numero_siguiendo);
        if(arreglo_usuarios[i].numero_siguiendo != 0){
            printf("\nSiguiendo a:  ");
            for(j=0;j<numero_usuarios;j++){
                if( arreglo_usuarios[i].lista_siguiendo[j] == 1 ){
                    printf("%d ",j+1);
                }
            }
        }

    }/*end for*/

    printf("\n\nFin de la información obtenida a partir del archivo %s\n\n", nombre_archivo);

    /*inicializar el arreglo para guardar la información de los clientes conectados*/
    clientesEstados = (estadosClientes*)malloc(numero_usuarios*sizeof(estadosClientes));
    if (clientesEstados == NULL) {
        perror("Memoria no alocada");
        exit(1);
    }/*end if*/

    for(i=0;i<numero_usuarios;i++){
        clientesEstados[i].pid = 0;
        clientesEstados[i].activo = false;
        strcpy(clientesEstados[i].pipe_cliente_a_servidor , "");
        strcpy(clientesEstados[i].pipe_servidor_a_cliente , "");
    }

    /*ejecutar hasta seleccionar desconexión*/
    while(1){

        creado = 0;
        do {
           id_pipe_inicial = open (pipeInicial, O_RDONLY);
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

    /*limpiar memoria*/
    /*for(i=0;i<numero_usuarios;i++){
        if(arreglo_usuarios[i].lista_siguiendo != NULL){
            free(arreglo_usuarios[i].lista_siguiendo);
            arreglo_usuarios[i].lista_siguiendo = NULL;
        }

        if(arreglo_usuarios[i].tweets != NULL){
            free(arreglo_usuarios[i].tweets);
            arreglo_usuarios[i].tweets = NULL;
        }/*end if
    }

    free(arreglo_usuarios);
    arreglo_usuarios = NULL;*/

    exit(0);

}
