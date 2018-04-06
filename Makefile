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

$(ODIR)/%.o: $(SRC)/%.c $(DEPS) subdir
	$(CC) -c -o $@ $< $(CFLAGS)
#$(ODIR)/%.o: $(SRC)/%.c $(DEPS) subdir
#        $(CC) -c -o $@ $< $(CFLAGS)

SUBDIRS = miniupnpc

all: tracer client checker

#client: $(OBJ_CLIENT) subdir
#        gcc -o $@ $(OBJ_CLIENT) $(CFLAGS) $(LIBS) -static -L$(SUBDIRS) -lminiupnpc 
client: $(OBJ_CLIENT) subdir
	gcc -o $@ $(OBJ_CLIENT) $(CFLAGS) $(LIBS) -Llib -lminiupnpc

#	client: $(OBJ_CLIENT) subdir
#        gcc -o $@ $(OBJ_CLIENT) $(CFLAGS) $(LIBS) -Llib -lminiupnpc

tracer: $(OBJ_TRACER)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

checker: $(OBJ_CHECKER)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

subdir:
	cd $(SUBDIRS) && $(MAKE)

.PHONY: clean all

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ client tracer checker
	cd $(SUBDIRS) && $(MAKE) clean
