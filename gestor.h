/*
Nombre del archivo: gestor.h
Autores: Adrián García y Luis Rosales
Objetivo: proveer las librerías, las constantes, los prototipos de las funciones
  y las variables globales (requeridas por varias funciones) que usa el proceso
  servidor.
Funciones: AdicionarSeguidor, RemoverSeguidor, Desconectar, GuardarTweet,
  EnviarTweetsASeguidorRecienConectado, EnviarTweetASeguidoresConectados,
  signalHandler, ManejarNuevaConexion, ImprimirInstruccionesComando,
  ImprimirInformacionEstructuraUsuarios, main
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
estados_clientes* clientes_estados;
usuario* arreglo_usuarios;
char* pipe_inicial;

typedef void (*sighandler_t)(int);

void AdicionarSeguidor(int numero_usuario_seguidor, int numero_usuario_a_seguir);
void RemoverSeguidor(int numero_usuario_seguidor, int numero_usuario_seguido);
void Desconectar(int i);
void GuardarTweet(mensaje_del_cliente mensaje_recibido);
void EnviarTweetsASeguidorRecienConectado(comunicacion_inicial_cliente nuevo_cliente);
void EnviarTweetASeguidoresConectados(mensaje_del_cliente mensaje_recibido);
sighandler_t ManejadorSenalOperaciones (void);
sighandler_t ManejadorSenalTerminacion (void);
sighandler_t ManejadorSenalDesconexion (void);
void ManejarNuevaConexion(comunicacion_inicial_cliente nuevo_cliente);
void ImprimirInstruccionesComando();
void ImprimirInformacionEstructuraUsuarios();
int main (int argc, char **argv);
