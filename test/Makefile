CC=aarch64-linux-gnu-gcc
CFLAGS=-static

all: test_ptree

test_ptree: test_ptree.c
	$(CC) $(CFLAGS) -o test_ptree test_ptree.c

clean:
	rm -f test_ptree