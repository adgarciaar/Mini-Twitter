/*
Nombre del archivo: gestor.h
Autores: Adrián García y Luis Rosales
Objetivo: proveer las librerías, las constantes, los prototipos de las funciones
  y las variables globales (requeridas por varias funciones) que usa el proceso
  servidor.
Funciones: adicionarSeguidor, removerSeguidor, desconectar, guardarTweet,
  EnviarTweetsASeguidorRecienConectado, enviarTweetASeguidoresConectados,
  signalHandler, manejarNuevaConexion, imprimirInstruccionesComando,
  imprimirInformacionEstructuraUsuarios, main
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
#include "procesamiento_archivo.h"

int numero_usuarios;
estadosClientes* clientesEstados;
usuario* arreglo_usuarios;
char* pipeInicial;

typedef void (*sighandler_t)(int);

void adicionarSeguidor(int numero_usuario_seguidor, int numero_usuario_a_seguir);
void removerSeguidor(int numero_usuario_seguidor, int numero_usuario_seguido);
void desconectar(int i);
void guardarTweet(mensajeDelCliente mensajeRecibido);
void EnviarTweetsASeguidorRecienConectado(comunicacionInicialCliente nuevoCliente);
void enviarTweetASeguidoresConectados(mensajeDelCliente mensajeRecibido);
sighandler_t signalHandler (void);
sighandler_t signalHandlerSalida (void);
void manejarNuevaConexion(comunicacionInicialCliente nuevoCliente);
void imprimirInstruccionesComando();
void imprimirInformacionEstructuraUsuarios();
int main (int argc, char **argv);
