/*
Nombre del archivo: estructuras_comunicacion.h
Autores: Adrián García y Luis Rosales
Objetivo: proveer las librerías, las constantes y las estructuras de datos
  para intercambiar datos a través de pipes, tanto para el servidor como para los
  clientes.
Funciones: ninguna.
Fecha de última modificación: 04/11/19
*/

#include <stdbool.h>

#define TAMANO_TWEET 201
#define TAMANO_NOMBRE_PIPE 35
#define TAMANO_STRING_ID_PROCESO 10
#define TAMANO_STRING_ID_USUARIO 10

typedef struct data {
    int pid;
    int numeroCliente;
    char pipe_cliente_a_servidor[TAMANO_NOMBRE_PIPE];
    char pipe_servidor_a_cliente[TAMANO_NOMBRE_PIPE];
} comunicacionInicialCliente;

typedef struct {
    int pid;
    int operacion;
    int idTweetero; /*si se envió tweet acá se envía el id de quién lo hizo*/
    char mensaje[TAMANO_TWEET];
} mensajeDelServidor;

typedef struct {
    int pid;
    int operacion; /*1 para follow, 2 unfollow, 3 tweet, 4 desconexión*/
    int numeroCliente;
    char mensaje[TAMANO_TWEET];
    int numero_cliente_follow_unfollow;
} mensajeDelCliente;

typedef struct {
    int pid;
    char pipe_cliente_a_servidor[TAMANO_NOMBRE_PIPE];
    char pipe_servidor_a_cliente[TAMANO_NOMBRE_PIPE];
    bool activo;
} estadosClientes;
