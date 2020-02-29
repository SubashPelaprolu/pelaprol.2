CC=gcc
CFLAGS=-Wall -ggdb -Werror
LDFLAGS=-pthread

all: oss user

common.o: common.c oss.h user.h
	$(CC) $(CFLAGS) -c common.c

oss: oss.c common.o
	$(CC) $(CFLAGS) oss.c common.o -o oss $(LDFLAGS)

user: user.c user.h common.o
	$(CC) $(CFLAGS) user.c common.o -o user $(LDFLAGS)

clean:
	rm -rf oss user common.o
