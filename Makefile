parser: parser.c scanner.c newsym.c turtle.c newsym.h turtle.h scanner.h
	gcc -g -std=c99 parser.c turtle.c newsym.c newsym.h scanner.c turtle.h scanner.h -o parser -lm
clean:
	rm -rf *.o *~ *.dSYM parser
