include ../Makefile.variables

all: build

build:
	$(CC) $(FLAGS) -c *.c
	ar rcs libdf.a ../parser/*.o *.o

clean:
	rm -f *.o *.a