SRC = main.c util.c
OBJ = $(SRC:.c=.o)
CC = cc
EXE = swissmatchup
CFLAGS = -pedantic -Wall -O2


.PHONY: all help clean

default: all

all: $(EXE)
	cp $(EXE) ../out

help:
	@echo "To compile, type:"
	@echo ""
	@echo "make [target]"
	@echo ""
	@echo "Where target is one of the following:"
	@echo ""
	@echo "all:            > Compile and link all source files"
	@echo "help:           > Print this message"
	@echo "clean:          > Clean up"
	@echo ""
	@echo "If no target is given, it will use \"all\""

clean:
	rm -f $(EXE) $(OBJ)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)
