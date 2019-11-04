all: gestor cliente

gestor: gestor.o procesamiento_archivo.o
		gcc -ansi -o gestor gestor.o procesamiento_archivo.o
gestor.o: gestor.c estructuras_comunicacion.h procesamiento_archivo.c procesamiento_archivo.h
		gcc -c gestor.c

cliente: cliente.o
		gcc -ansi -o cliente cliente.o
cliente.o: cliente.c estructuras_comunicacion.h
		gcc -c cliente.c

clean:
		rm *.o gestor cliente
