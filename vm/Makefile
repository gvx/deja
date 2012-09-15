all: release debug
release: vu std.vu
debug: vu-dbg std.vu

clean:
	rm *.o

CFILES = $(wildcard *.c)
OFILES = $(patsubst %.c, %.o, $(CFILES))
ODBGFILES = $(patsubst %.c, %.dbg.o, $(CFILES))

vu: $(OFILES)
	$(CC) -O2 $(CFLAGS) -Wall -ldl -lm $^ -o $@

%.o: %.c %.h
	$(CC) -O2 $(CFLAGS) -Wall -c -o $@ $<

vu.o: vu.c
	$(CC) -O2 $(CFLAGS) -Wall -c -o $@ $<

vu-dbg: $(ODBGFILES)
	$(CC) -ggdb3 $(CFLAGS) -Wall -ldl -lm $^ -o $@

%.dbg.o: %.c %.h
	$(CC) -ggdb3 $(CFLAGS) -Wall -c -o $@ $<

vu.dbg.o: vu.c
	$(CC) -ggdb3 $(CFLAGS) -Wall -c -o $@ $<

std.vu: std.dva
	python ../dvasm.py < $^ > $@

std.c: std.vu
	python ../genstdc.py
