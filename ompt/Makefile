CC = gcc
ROOT = /home/idriss/Work/Software/llvm-project/build-openmp

CFLAGS = -ggdb -shared -fPIC -fopenmp -I${ROOT}/include -Iinclude -Izmq
LFLAGS = -lzmq -lpapi

LIBRARY = libTPMompt.so

SOURCES = trace.c

all: $(LIBRARY)

$(LIBRARY): $(SOURCES)
	$(CC) $(CFLAGS) -o $(LIBRARY) $(SOURCES) $(LFLAGS)

clean:
	rm -f $(LIBRARY)
