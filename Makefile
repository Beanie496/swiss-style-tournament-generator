SRC = main.c util.c
OBJ = $(SRC:.c=.o)
BINDIR = /usr/local/bin
CC = cc
STRIP = strip
EXE = swissmatchup
CFLAGS = -pedantic -Wall -O2


.PHONY: all help strip install clean

default: all

all: $(OBJ)

help:
	@echo "To compile, type:"
	@echo ""
	@echo "make [target]"
	@echo ""
	@echo "Where target is one of the following:"
	@echo ""
	@echo "all:            > Compile and link all source files"
	@echo "help:           > Print this message"
	@echo "strip:          > Strip the executable"
	@echo "install:        > Strip and install the executable"
	@echo "uninstall:      > Uninstall the executable"
	@echo "clean:          > Clean up"
	@echo ""
	@echo "If no target is given, it will use \"all\""

strip:
	$(STRIP) $(EXE)

install: strip
	-mkdir -p -m 755 $(BINDIR)
	-cp $(EXE) $(BINDIR)

uninstall:
	rm -f $(BINDIR)/$(EXE)

clean:
	rm -f $(EXE) $(OBJ)

$(EXE): $(OBJ)
	+$(CC) $(CFLAGS) -o $@ $(OBJ)
