all: buildLib buildArgs link

buildLib:
	cd dataflow && make && cd ..
	cd domains && make && cd ..

buildArgs:
	cd argparse && make && cd ..

link:
	gcc argparse/argparse.o domains/util.o domains/concrete.o domains/abstract.o main.c -o unrelyzer -Ldataflow -ldf

clean:
	cd dataflow && make clean && cd ..
	cd domains && make clean && cd ..
	cd argparse && make clean && cd ..
	rm -f unrelyzer *.dot *.png