#include <stdbool.h>

typedef struct data {
    int pid;
    int numeroCliente;
    char pipe_cliente_a_servidor[30];
    char pipe_servidor_a_cliente[30];
} comunicacionInicialCliente;

typedef struct {
    int pid;
    int idTweetero;
    int numeroMensajes;
    char mensaje[201];
} mensajeDelServidor;

typedef struct {
    int pid;
    int operacion; //1 para follow, 2 unfollow, 3 tweet, 4 desconexion
    int numeroCliente;
    char mensaje[201];
} mensajeDelCliente;

typedef struct {
    int pid;
    char pipe_cliente_a_servidor[30];
    char pipe_servidor_a_cliente[30];
    bool activo;
} estadosClientes;
