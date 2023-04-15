calculator: calculator.o tokenize.o
	gcc bin/calculator.o bin/tokenize.o -o bin/calculator -g -lm

clean: 
	rm -rf bin/

calculator.o: calculator.c
	mkdir -p bin/
	gcc calculator.c -c -o bin/calculator.o -g

tokenize.o: tokenize.c tokenize.h
	mkdir -p bin/
	gcc tokenize.c -c -o bin/tokenize.o -g