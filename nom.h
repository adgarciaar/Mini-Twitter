#include <stdbool.h>

typedef struct data {
    int pid;
    int numeroCliente;
    char pipe_cliente_a_servidor[30];
    char pipe_servidor_a_cliente[30];
} comunicacionInicialCliente;

typedef struct {
    int pid;
    int numeroMensajes;
    char mensaje[200];
} mensajeDelServidor;

typedef struct {
    int pid;
    bool desconexion;
    int numeroCliente;
    char mensaje[200];
} mensajeDelCliente;

typedef struct {
    int pid;
    char pipe_cliente_a_servidor[30];
    char pipe_servidor_a_cliente[30];
    int activo;
} estadosClientes;
