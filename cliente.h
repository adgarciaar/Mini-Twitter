/*
Nombre del archivo: cliente.h
Autores: Adrián García y Luis Rosales
Objetivo: proveer las librerías, los prototipos de las funciones y las variables
  globales (requeridas por varias funciones) que usa cada proceso cliente.
Funciones: ManejadorSenal, follow, Unfollow, EnviarTweet, Desconectar,
  ImprimirInstruccionesComando, main
Fecha de última modificación: 04/11/19
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "estructuras_comunicacion.h"

int id_pipe_cliente_a_servidor, id_pipe_servidor_a_cliente;
char pipe_cliente_a_servidor[TAMANO_NOMBRE_PIPE] = "pipeCliente";
char pipe_servidor_a_cliente[TAMANO_NOMBRE_PIPE] = "pipeCliente";
bool cliente_aceptado;
int pid_servidor;
int id_cliente;

typedef void (*sighandler_t)(int);

sighandler_t ManejadorSenal (void);
void Follow();
void Unfollow();
void EnviarTweet();
void Desconectar(char* pipe_inicial);
void ImprimirInstruccionesComando();
int main (int argc, char **argv);
