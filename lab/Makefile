CC      = gcc
CFLAGS  = -Wall -Wextra -pthread

all: chat_serverd chat_client

chat_serverd: server.c common.h
	$(CC) $(CFLAGS) -o $@ server.c

chat_client: client.c common.h
	$(CC) $(CFLAGS) -o $@ client.c

check: chat_serverd
	./tests/run_tests.sh

clean:
	rm -f chat_serverd chat_client

.PHONY: all check clean
