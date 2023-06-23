CC = gcc
ROOT = /home/cc
OPENMP = ${ROOT}/llvm-project/build-openmp

CFLAGS = -O2 -shared -fPIC -fopenmp -I${OPENMP}/include -Iinclude -Izmq
LFLAGS = -lzmq -lpapi

LIBRARY = libTPMompt.so

SOURCES = trace.c

all: $(LIBRARY)

$(LIBRARY): $(SOURCES)
	$(CC) $(CFLAGS) -o $(LIBRARY) $(SOURCES) $(LFLAGS)

clean:
	rm -f $(LIBRARY)
