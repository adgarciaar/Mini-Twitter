/*
Nombre del archivo: procesamiento_archivo.c
Autores: Adrián García y Luis Rosales
Objetivo: implementar las funciones que leen el archivo de relaciones de
  los usuarios y a partir de éste generar un arreglo con la información de
  los usuarios.
Funciones: AbrirArchivo, ContarLineasArchivo, LeerArchivo
Fecha de última modificación: 04/11/19
*/

#include "procesamiento_archivo.h"

/*
Función: AbrirArchivo
Autores de la función: Luis Rosales.
Parámetros de entrada: nombre del archivo a abrir.
Retorno: un valor booleano que es true si el archivo se puede abrir
o false si el archivo no se puede abrir.
Descripción: verifica que un archivo de entrada se puede abrir.
*/
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

/*
Función:ContarLineasArchivo
Autores de la función: Luis Rosales.
Parámetros de entrada: nombre del archivo al que se le van a contar las líneas.
Retorno: el número de líneas que tiene el archivo.
Descripción: cuenta el número de líneas que tiene un archivo y retorna dicho valor.
*/
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

/*
Función: LeerArchivo
Autores de la función: Adrián García y Luis Rosales.
Parámetros de entrada: nombre del archivo a leer y número de líneas que tiene.
Retorno: apuntador a arreglo de tipo usuario, que contiene todos los datos de
los usuarios, extraídos del archivo.
Descripción: lee un archivo línea por línea, almacenando los elementos de cada
una en una estructura usuario (definida en procesamientoArchivo.h) y devuelve
un apuntador a arreglo de tipo usuario que contiene todos los datos de los
usuarios.
*/

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
        }/*end if*/
        arreglo_usuarios[i].tweets = NULL;
        arreglo_usuarios[i].numero_tweets = 0;
    }/*end for*/

    archivo = fopen( nombre_archivo ,"r");

    if ( archivo != NULL ){

      /* Get the first line of the file. */
      line_size = getline(&linea, &line_buf_size, archivo);
      while (line_size >= 0){

          strcpy(linea_aux, linea);

          for(i = 0; i <= strlen(linea_aux); i++){
              if(linea_aux[i] == delimitador2){
                  linea_aux[i] = ' ';
              }/*end if*/
          }/*end for*/

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
              }/*end if*/
              contador_horizontal = contador_horizontal + 1;
              token = strtok(NULL, delimitador);
          }/*end while*/
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
