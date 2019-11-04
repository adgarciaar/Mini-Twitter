/*
Nombre del archivo:
Autores: Adrián García y Luis Rosales
Objetivo:
Funciones:
Fecha de última modificación:
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAMANO_MAXIMO_TWEET 201
#define TAMANO_MAXIMO_LINEA 21

typedef struct {
    int id;
    int numero_siguiendo;
    int* lista_siguiendo;
    char** tweets;
} usuario ;

bool AbrirArchivo(char nombre_archivo[]);
int ContarLineasArchivo(char nombre_archivo[]);
usuario* LeerArchivo(char nombre_archivo[], int numero_lineas_archivo);
