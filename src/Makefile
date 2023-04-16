calculator: calculator.o calcparser.o tokenize.o
	gcc bin/calculator.o bin/calcparser.o bin/tokenize.o -o bin/calculator -g

clean: 
	rm -rf bin/

calculator.o: calculator.c
	mkdir -p bin/
	gcc calculator.c -c -o bin/calculator.o -g

tokenize.o: tokenize.c tokenize.h
	mkdir -p bin/
	gcc tokenize.c -c -o bin/tokenize.o -g

calcparser.o: calcparser.c calcparser.h
	mkdir -p bin/
	gcc calcparser.c -c -o bin/calcparser.o -g