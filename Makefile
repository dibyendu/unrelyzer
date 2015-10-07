all:
	cd src && make all && cd ..
	mkdir -p build
	cp src/parser src/input build
clean:
	cd src && make clean && cd ..
	rm -rf build

