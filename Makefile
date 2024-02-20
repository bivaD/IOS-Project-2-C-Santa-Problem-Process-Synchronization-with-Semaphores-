LC_ALL=cs_CZ.utf8
CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -pedantic -pthread

# $^ nahradi se prerekvizitami
# $@ nahardi se targetem

all: proj2

proj2: proj2.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm proj2

zip: all
	zip proj2.zip proj2.c Makefile

