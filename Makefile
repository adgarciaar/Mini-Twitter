all: algo

algo: algo.o
		gcc -ansi -o algo algo.o
algo.o: algo.c algo.h
		gcc -c algo.c

clean:
		rm *.o algo
