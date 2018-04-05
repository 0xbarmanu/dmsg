IDIR=include
SRC=src
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
#LDIR =./lib

LIBS=-lm

_DEPS = common.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ_TRACER = common.o tracer.o 
OBJ_TRACER = $(patsubst %,$(ODIR)/%,$(_OBJ_TRACER))

_OBJ_CLIENT = common.o client.o  
OBJ_CLIENT = $(patsubst %,$(ODIR)/%,$(_OBJ_CLIENT))

_OBJ_CHECKER = common.o checker.o 
OBJ_CHECKER = $(patsubst %,$(ODIR)/%,$(_OBJ_CHECKER))

$(ODIR)/%.o: $(SRC)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: tracer client checker


client: $(OBJ_CLIENT)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

tracer: $(OBJ_TRACER)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

checker: $(OBJ_CHECKER)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean all

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ client tracer checker