all: biuldLib build

biuldLib:
	cd dataflow && make && cd ..

build:
	gcc domains/util.c domains/concrete/concrete.c domains/abstract/abstract.c main.c -o analyzer -Ldataflow -ldf

clean:
	cd dataflow && make clean && cd ..
	rm -f analyzer *.dot *.png
