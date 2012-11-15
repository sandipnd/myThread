CFLAGS=-Wall  -g

# To debug with valgrind:
#CFLAGS:=$(CFLAGS) -DVALGRIND

# To get debugging output
#CFLAGS:=$(CFLAGS) -DLF_DEBUG

#PROGRAMS=basic-uc basic-sjlj basic-clone example-uc example-sjlj example-clone example-asm
PROGRAMS=example-mt

all: $(PROGRAMS)

clean:
	$(RM) *.o $(PROGRAMS) &> /dev/null || true
	
debug: clean
	make "CC=gcc -g"

example-mt: mt.o dphil_skeleton.o
	$(CC) $(LDFLAGS) mt.o dphil_skeleton.o -o example-mt

mt.o: mt.h
dphil_skeleton.o: mt.h
