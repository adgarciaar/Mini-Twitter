/*
Nombre del archivo: cliente.h
Autores: Adrián García y Luis Rosales
Objetivo: proveer las librerías, los prototipos de las funciones y las variables
  globales (requeridas por varias funciones) que usa cada proceso cliente.
Funciones: signalHandler, follow, unfollow, enviarTweet, desconectar,
  imprimirInstruccionesComando, main
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
char mensaje[TAMANO_TWEET];
char pipe_cliente_a_servidor[TAMANO_NOMBRE_PIPE] = "pipeCliente";
char pipe_servidor_a_cliente[TAMANO_NOMBRE_PIPE] = "pipeCliente";
bool clienteAceptado;
int pidServidor;
int idCliente;

typedef void (*sighandler_t)(int);

sighandler_t signalHandler (void);
void follow();
void unfollow();
void enviarTweet();
void desconectar();
void imprimirInstruccionesComando();
int main (int argc, char **argv);
