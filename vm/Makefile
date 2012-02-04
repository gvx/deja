all: vu
debug: vu-dbg

clean:
	rm *.o

CFILES = $(wildcard *.c)
OFILES = $(patsubst %.c, %.o, $(CFILES))

vu: $(OFILES)
	$(CC) -Wall -ldl -lm $^ -o $@

vu-dbg: $(OFILES)
	$(CC) -ggdb -Wall -ldl -lm $^ -o $@

%.o: %.c %.h
		$(CC) -c -o $@ $< $(CFLAGS)
