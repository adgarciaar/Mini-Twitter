all: gestor cliente

gestor: gestor.o
		gcc -ansi -o gestor gestor.o
gestor.o: gestor.c nom.h
		gcc -c gestor.c

cliente: cliente.o
		gcc -ansi -o cliente cliente.o
cliente.o: cliente.c nom.h
		gcc -c cliente.c

clean:
		rm *.o gestor cliente
