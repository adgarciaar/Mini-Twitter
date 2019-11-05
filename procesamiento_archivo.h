/*
Nombre del archivo: procesamiento_archivo.h
Autores: Adrián García y Luis Rosales
Objetivo: proveer las librerías, las constantes y los prototipos de las funciones
  para leer el archivo de relaciones y a partir de éste crear un arreglo con la
  información de los usuarios. Aquí también se provee la estructura para almacenar
  la información de cada usuario.
Funciones: AbrirArchivo, ContarLineasArchivo, LeerArchivo
Fecha de última modificación: 04/11/19
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAMANO_NOMBRE_ARCHIVO 35
#define TAMANO_MAXIMO_LINEA 21

typedef struct {
    int id;
    int numero_siguiendo;
    int* lista_siguiendo;
    char** tweets;
    int numero_tweets;
} usuario ;

bool AbrirArchivo(char nombre_archivo[]);
int ContarLineasArchivo(char nombre_archivo[]);
usuario* LeerArchivo(char nombre_archivo[], int numero_lineas_archivo);
