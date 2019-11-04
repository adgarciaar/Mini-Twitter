/*
Nombre del archivo:
Autores: Adrián García y Luis Rosales
Objetivo:
Funciones:
Fecha de última modificación:
*/

#include "procesamiento_archivo.h"

bool AbrirArchivo(char nombre_archivo[]){
    FILE *archivo;
    archivo = fopen( nombre_archivo ,"r");
    if ( archivo != NULL ){
        fclose(archivo);
        return true;
    }else{
        perror ( nombre_archivo );
        return false;
    }/*end if*/
}

int ContarLineasArchivo(char nombre_archivo[]){
    FILE *archivo;
    int contador_lineas = 0;
    char caracter;

    archivo = fopen(nombre_archivo, "r");

    if (archivo == NULL){
        perror ( nombre_archivo );
        return 0;
    }/*end if*/

    /* extraer caracteres del archivo y almacenarlos en variable caracter*/
    for (caracter = getc(archivo); caracter != EOF; caracter = getc(archivo)){
        if (caracter == '\n'){
            contador_lineas = contador_lineas + 1;
        }/*end if*/
    }/*end for*/

    fclose(archivo);

    return contador_lineas;
}

usuario* LeerArchivo(char nombre_archivo[], int numero_lineas_archivo){

    /*printf("%d\n", numero_lineas_archivo);*/

    FILE *archivo;
    //char linea[TAMANO_MAXIMO_LINEA];
    char *linea = NULL;
    char linea_aux[TAMANO_MAXIMO_LINEA];
    char delimitador[] = " ";
    int numero_linea = 0, contador = 0;
    int i = 0;
    char *token;
    char *caracter_aux;
    usuario* arreglo_usuarios = (usuario*)malloc(numero_lineas_archivo*sizeof(usuario));
    int* arreglo_auxiliar = NULL;
    int contador_horizontal = 0;
    char delimitador2 = '\t';
    int posicion;
    ssize_t line_size;
    char *line_buf = NULL;
    size_t line_buf_size = 0;

    if (arreglo_usuarios == NULL) {
        perror("Memoria no alocada");
        exit(1);
    }/*end if*/

    for(i=0;i<numero_lineas_archivo;i++){
        arreglo_usuarios[i].id = i;
        arreglo_usuarios[i].numero_siguiendo = 0;
        /*printf("%d\n", arreglo_usuarios[i].numero_siguiendo);*/
        arreglo_usuarios[i].lista_siguiendo = (int*)malloc(numero_lineas_archivo*sizeof(int));
        if(arreglo_usuarios[i].lista_siguiendo == NULL){
            perror("Memoria no alocada");
            exit(1);
        }
        arreglo_usuarios[i].tweets = NULL;
        arreglo_usuarios[i].numero_tweets = 0;
    }

    archivo = fopen( nombre_archivo ,"r");

    if ( archivo != NULL ){

      /* Get the first line of the file. */
      line_size = getline(&linea, &line_buf_size, archivo);
      while (line_size >= 0){

          strcpy(linea_aux, linea);

          for(i = 0; i <= strlen(linea_aux); i++){
              if(linea_aux[i] == delimitador2){
                  linea_aux[i] = ' ';
              }
          }

          contador_horizontal = 0;

          if( (caracter_aux = strchr(linea_aux, '\n')) != NULL){
              *caracter_aux = '\0';
          }/*end if*/


          token = strtok(linea_aux, delimitador);
          while (token != NULL) {

              if(atoi(token)==1){
                  arreglo_usuarios[numero_linea].lista_siguiendo[contador_horizontal] = 1;
                  arreglo_usuarios[numero_linea].numero_siguiendo = arreglo_usuarios[numero_linea].numero_siguiendo+1;
              }else{
                  arreglo_usuarios[numero_linea].lista_siguiendo[contador_horizontal] = 0;
              }
              contador_horizontal = contador_horizontal + 1;
              token = strtok(NULL, delimitador);
          }
          numero_linea = numero_linea + 1;
          /* Get the next line */
          line_size = getline(&linea, &line_buf_size, archivo);

      }/*end while*/
      fclose(archivo);

    }else{
       free(arreglo_usuarios);
       perror ( nombre_archivo );
       exit(1);
    }/*end if*/

    return arreglo_usuarios;

}

/*int main (int argc, char **argv) {
    /*usar realloc() para cambiar tamaño en ejecución
    int i,j;
    int numero_lineas_archivo = 0, numero_usuarios;
    int numero_siguiendo = 0;

    char nombre_archivo[] = "matriz.txt";

    if( AbrirArchivo(nombre_archivo) == false ){
        exit(-1);
    }

    numero_lineas_archivo = ContarLineasArchivo(nombre_archivo);

    usuario* arreglo_usuarios = LeerArchivo(nombre_archivo, numero_lineas_archivo);
    numero_usuarios = numero_lineas_archivo;

    for(i=0;i<numero_usuarios;i++){

        printf("\n\n%s%d\n", "Usuario ",i+1);
        printf("%s%d\n", "Siguiendo a ",arreglo_usuarios[i].numero_siguiendo);
        printf("lista_siguiendo:");

        for(j=0;j<arreglo_usuarios[i].numero_siguiendo;j++){
            printf("%d ", arreglo_usuarios[i].lista_siguiendo[j]+1);
        }

        if(arreglo_usuarios[i].lista_siguiendo != NULL){
            free(arreglo_usuarios[i].lista_siguiendo);
            arreglo_usuarios[i].lista_siguiendo = NULL;
        }

        if(arreglo_usuarios[i].tweets != NULL){
            free(arreglo_usuarios[i].tweets);
            arreglo_usuarios[i].tweets = NULL;
        }/*end if
    }/*end for

    free(arreglo_usuarios);
    arreglo_usuarios = NULL;

    printf("\n");

    return(0);
}*/
