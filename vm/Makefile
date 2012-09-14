all: vu
all: CC += -O2
all: std.vu
debug: vu-dbg
debug: CC += -ggdb3
debug: std.vu

clean:
	rm *.o

CFILES = $(wildcard *.c)
OFILES = $(patsubst %.c, %.o, $(CFILES))

vu vu-dbg: $(OFILES)
	$(CC) -Wall -ldl -lm $^ -o $@

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

std.vu: std.dva
	python ../dvasm.py < $^ > $@

std.c: std.vu
	python ../genstdc.py
