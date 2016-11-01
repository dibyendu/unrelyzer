include ../Makefile.variables

all: buildConcrete buildAbstract
	$(CC) $(FLAGS) -c *.c

buildConcrete:
	$(CC) $(FLAGS) -c concrete/*.c

buildAbstract:
	$(CC) $(FLAGS) -c abstract/*.c

clean:
	rm -f *.o