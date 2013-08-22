.PHONY: all release debug clean depend

release: vu
all: release debug
debug: vu-dbg

clean:
	rm *.o

CFILES := $(wildcard *.c) $(shell if [ ! -e "std.c" ]; then echo "std.c"; fi)
OFILES = $(patsubst %.c, %.o, $(CFILES))
ODBGFILES = $(patsubst %.c, %.dbg.o, $(CFILES))
CFLAGS = -Wall -falign-functions=4
LDFLAGS = -ldl -lm

vu: .depend $(OFILES)
	$(CC) -O2 $(CFLAGS) $(OFILES) -o $@ $(LDFLAGS)

vu-dbg: .depend.dbg $(ODBGFILES)
	$(CC) -ggdb3 $(CFLAGS) $(ODBGFILES) -o $@ $(LDFLAGS)

depend: .depend .depend.dbg

.depend: cmd = gcc -MM -MF depend $(var); cat depend >> .depend;
.depend:
	@echo "Generating dependencies..."
	@$(foreach var, $(CFILES), $(cmd))
	@rm -f depend

.depend.dbg: cmd = gcc -MM -MF depend $(var); sed 's/\.o/.dbg.o/' < depend >> .depend.dbg;
.depend.dbg:
	@echo "Generating dependencies..."
	@$(foreach var, $(CFILES), $(cmd))
	@rm -f depend

-include .depend
-include .depend.dbg

%.o: %.c
	$(CC) -O2 $(CFLAGS) -c -o $@ $<

%.dbg.o: %.c
	$(CC) -ggdb3 $(CFLAGS) -c -o $@ $<

std.vu: std.dva
	python ../dvasm.py < $^ > $@

std.c: std.vu
	python ../genstdc.py
