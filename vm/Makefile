all: vu
all: std.vu
debug: vu-dbg
debug: CC += -ggdb
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
