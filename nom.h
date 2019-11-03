typedef struct data {
    int pid;
    int numeroCliente;
    char pipeEspecifico[20];
} infoPipe;

typedef struct {
    int pid;
    int numeroMensajes;
    char mensaje[200];
} mensajeDelServidor;

typedef struct {
    int pid;
    int numeroCliente;
    char mensaje[200];
} mensajeDelCliente;
