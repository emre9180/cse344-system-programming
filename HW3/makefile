CC = gcc
CFLAGS = -Wall -Wextra -g

# Link with the real-time library
LDFLAGS = -pthread -lrt

# Source files
SRCS = main_client.c main_server.c client_info_queue.c common.c

# Object files
OBJS = $(SRCS:.c=.o)

all: main_client main_server

main_client: main_client.o client_info_queue.o common.o
	$(CC) $(CFLAGS) -o main_c main_client.o client_info_queue.o common.o $(LDFLAGS)

main_server: main_server.o client_info_queue.o common.o
	$(CC) $(CFLAGS) -o main_s main_server.o client_info_queue.o common.o $(LDFLAGS)

main_client.o: main_client.c
	$(CC) $(CFLAGS) -c main_client.c

main_server.o: main_server.c
	$(CC) $(CFLAGS) -c main_server.c

client_info_queue.o: client_info_queue.c
	$(CC) $(CFLAGS) -c client_info_queue.c

common.o: common.c
	$(CC) $(CFLAGS) -c common.c

clean:
	rm -f main_c main_s $(OBJS)
