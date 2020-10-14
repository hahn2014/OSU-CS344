CC=gcc
CFLAGS=-I.
DEPS = main.c
OBJ = movieDirs.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

movieDirs: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
