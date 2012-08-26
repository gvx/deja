all: vu
debug: vu-dbg
debug: CC += -ggdb

clean:
	rm *.o

CFILES = $(wildcard *.c)
OFILES = $(patsubst %.c, %.o, $(CFILES))

vu vu-dbg: $(OFILES)
	$(CC) -Wall -ldl -lm $^ -o $@

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)
