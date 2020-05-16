.PHONY: default
default: compile ;

CC=gcc
CFLAGS= -g -Wall -pedantic -I.
CompileElements  = supermercato.o boss.o cliente.o log.o code.o messaggi.o cassa.o signal.o

compile: $(CompileElements)
	$(CC) $(CompileElements) -g -lm -o exe.o -lpthread

supermercato.o: supermercato.c
boss.o: boss.c
cliente.o: cliente.c
log.o: log.c
code.o: code.c
messaggi.o: messaggi.c
cassa.o: cassa.c
signal.o: signal.c

test: exe.o
	chmod +x ./valgrind.sh
	./valgrind.sh

test1: exe.o
	chmod +x ./valgrind.sh
	./valgrind.sh

test2: exe.o
	chmod +x ./valgrind.sh
	./valgrind.sh

valgrind: exe.o
	chmod +x ./valgrind.sh
	./valgrind.sh
	
clean: clean.sh
	chmod +x ./clean.sh
	./clean.sh

analisi: analisi.sh
	chmod +x ./analisi.sh
	./analisi.sh

help: 
	@echo make 			- Compila il programma;
	@echo make compile 	- Compila il programma;
	@echo make clean 	- Cancella gli eseguibili;
	@echo make test 	- Esegue il test su test1.txt;
	@echo make test1 	- Esegue il test su test1.txt;
	@echo make test2 	- Esegue il test su test1.txt;
	@echo make valgrind - Esegue il test su test1.txt;
	@echo make analisi 	- Avvia lo script di analisi;
