/*
Nombre del archivo:
Autores: Adrián García y Luis Rosales
Objetivo:
Funciones:
Fecha de última modificación:
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

#define TAMMENSAJE 200

int id_pipe_cliente_a_servidor, id_pipe_servidor_a_cliente;
char mensaje[TAMMENSAJE];
char pipe_cliente_a_servidor[30] = "pipeCliente";
char pipe_servidor_a_cliente[30] = "pipeCliente";
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
