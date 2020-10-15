# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2020-21

CC   = gcc
LD   = gcc
CFLAGS =-Wall -std=gnu99 -I../
LDFLAGS=-lm -pthread

# A phony target is one that is not really the name of a file
# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean run

all: tecnicofs

tecnicofs: fs/state.o fs/operations.o synch.o main.o
	$(LD) $(CFLAGS) $(LDFLAGS) -o tecnicofs fs/state.o fs/operations.o synch.o main.o

synch.o: synch.c synch.h
	$(CC) $(CFLAGS) -o synch.o -c synch.c

fs/state.o: fs/state.c fs/state.h tecnicofs-api-constants.h synch.h
	$(CC) $(CFLAGS) -o fs/state.o -c fs/state.c

fs/operations.o: fs/operations.c fs/operations.h fs/state.h tecnicofs-api-constants.h 
	$(CC) $(CFLAGS) -o fs/operations.o -c fs/operations.c

main.o: main.c fs/operations.h fs/state.h tecnicofs-api-constants.h
	$(CC) $(CFLAGS) -o main.o -c main.c

clean:
	@echo Cleaning...
	rm -f fs/*.o *.o tecnicofs

run: tecnicofs
	./tecnicofs

run1: tecnicofs
	./tecnicofs < inputs/test4.txt