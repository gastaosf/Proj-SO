# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2020-21

CC   = gcc
LD   = gcc
CFLAGS =-Wall -std=gnu99 -I../ -g
LDFLAGS=-lm -pthread

# A phony target is one that is not really the name of a file
# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean run

all: tecnicofs

tecnicofs: state.o operations.o main.o
	$(LD) $(CFLAGS) $(LDFLAGS) -o tecnicofs state.o operations.o main.o

state.o: state.c state.h ../tecnicofs-api-constants.h 
	$(CC) $(CFLAGS) -o state.o -c state.c

operations.o: operations.c operations.h state.h ../tecnicofs-api-constants.h 
	$(CC) $(CFLAGS) -o operations.o -c operations.c

main.o: main.c operations.h state.h ../tecnicofs-api-constants.h
	$(CC) $(CFLAGS) -o main.o -c main.c

clean:
	@echo Cleaning...
	rm -f *.o *.o tecnicofs

run: tecnicofs
	./tecnicofs

run1: tecnicofs
	./tecnicofs < inputs/test4.txt