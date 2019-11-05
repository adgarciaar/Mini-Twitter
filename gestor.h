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
#include "procesamiento_archivo.h"

#define TAMNOM 201

int numero_usuarios;
estadosClientes* clientesEstados;
usuario* arreglo_usuarios;

typedef void (*sighandler_t)(int);

void adicionarSeguidor(int numero_usuario_seguidor, int numero_usuario_a_seguir);
void removerSeguidor(int numero_usuario_seguidor, int numero_usuario_seguido);
void desconectar(int i);
void guardarTweet(mensajeDelCliente mensajeRecibido);
void EnviarTweetsASeguidorRecienConectado(comunicacionInicialCliente nuevoCliente);
void enviarTweetASeguidoresConectados(mensajeDelCliente mensajeRecibido);
sighandler_t signalHandler (void);
void manejarNuevaConexion(comunicacionInicialCliente nuevoCliente);
void imprimirInstruccionesComando();
void imprimirInformacionEstructuraUsuarios();
int main (int argc, char **argv);
