#include <stdbool.h>

#define TAMANO_TWEET

typedef struct data {
    int pid;
    int numeroCliente;
    char pipe_cliente_a_servidor[30];
    char pipe_servidor_a_cliente[30];
} comunicacionInicialCliente;

typedef struct {
    int pid;
    int operacion; 
    int idTweetero; /*si se envió tweet acá se envía el id de quien lo hizo*/
    char mensaje[201];
} mensajeDelServidor;

typedef struct {
    int pid;
    int operacion; /*1 para follow, 2 unfollow, 3 tweet, 4 desconexion*/
    int numeroCliente;
    char mensaje[201];
    int numero_cliente_follow_unfollow;
} mensajeDelCliente;

typedef struct {
    int pid;
    char pipe_cliente_a_servidor[30];
    char pipe_servidor_a_cliente[30];
    bool activo;
} estadosClientes;
