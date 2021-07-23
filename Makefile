.POSIX:

CC = cc

all:
	$(CC) -o among-sus $(CFLAGS) main.c $(LDFLAGS)

clean:
	rm -f among-sus
