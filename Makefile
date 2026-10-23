CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2

SRC_LIB = src/colis.c src/tri.c src/camion.c
HDR     = src/colis.h src/tri.h src/camion.h

# Programme principal 
all: livraisons

livraisons: src/main.c $(SRC_LIB) $(HDR)
	$(CC) $(CFLAGS) -o livraisons src/main.c $(SRC_LIB)

# Programmes de test
test1: src/teste1.c $(SRC_LIB) $(HDR)
	$(CC) $(CFLAGS) -o test1 src/teste1.c $(SRC_LIB)

test2: src/teste2.c $(SRC_LIB) $(HDR)
	$(CC) $(CFLAGS) -o test2 src/teste2.c $(SRC_LIB)

test3: src/teste3.c $(SRC_LIB) $(HDR)
	$(CC) $(CFLAGS) -o test3 src/teste3.c $(SRC_LIB)

test4: src/teste4.c $(SRC_LIB) $(HDR)
	$(CC) $(CFLAGS) -o test4 src/teste4.c $(SRC_LIB)

tests: test1 test2 test3 test4
	./test1 > /dev/null && ./test2 > /dev/null && ./test3 > /dev/null && ./test4 > /dev/null && echo "Tous les tests passent."

# Mesures de performance
bench: src/bench.c $(SRC_LIB) $(HDR)
	$(CC) $(CFLAGS) -o bench src/bench.c $(SRC_LIB)

clean:
	rm -f livraisons test1 test2 test3 test4 bench