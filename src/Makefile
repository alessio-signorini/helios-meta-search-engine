CC = gcc
DEBUG_FLAGS = -g3 -Wall -pg
OPT_FLAGS = -O3
FILES = bench.c utils.c core.c socketutils.c searchengines.c parser.c cmdline.c helios.c

all: clean
	$(CC) $(OPT_FLAGS) *.c -o ../helios

clean:
	rm -f *~
