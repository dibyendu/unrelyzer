all: biuldLib build

biuldLib:
	cd dataflow && make && cd ..

build:
	gcc concrete.c main.c -o main -Ldataflow -ldf

clean:
	cd dataflow && make clean && cd ..
	rm -f main *.dot *.png
