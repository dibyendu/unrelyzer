include Makefile.variables

all: buildLib buildArgs link

buildLib:
	cd parser && make && cd ..
	cd dataflow && make && cd ..
	cd domains && make && cd ..

buildArgs:
	cd argparse && make && cd ..

link:
	$(CC) $(FLAGS) -c *.c
	$(CC) $(FLAGS) argparse/*.o domains/*.o *.o -o unrelyzer -Ldataflow -ldf

clean:
	cd parser && make clean && cd ..
	cd dataflow && make clean && cd ..
	cd domains && make clean && cd ..
	cd argparse && make clean && cd ..
	rm -f unrelyzer *.o *.dot *.png