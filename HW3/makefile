CC = gcc
CFLAGS = -Wall -Wextra -g

# Link with the real-time library
LDFLAGS = -pthread -lrt

# Source files
SRCS = Client/main_client.c Server/main_server.c Util/client_info_queue.c Util/common.c Sync/synch.c

# Object files
OBJS = $(SRCS:.c=.o)

all: main_client main_server

main_client: Client/main_client.o Util/client_info_queue.o Util/common.o synch.o
	$(CC) $(CFLAGS) -o main_c Client/main_client.o Util/client_info_queue.o common.o synch.o $(LDFLAGS)

main_server: Server/main_server.o client_info_queue.o common.o synch.o
	$(CC) $(CFLAGS) -o main_s Server/main_server.o client_info_queue.o common.o synch.o $(LDFLAGS)

main_client.o: Client/main_client.c
	$(CC) $(CFLAGS) -c Client/main_client.c

main_server.o: Server/main_server.c
	$(CC) $(CFLAGS) -c Server/main_server.c

client_info_queue.o: Util/client_info_queue.c
	$(CC) $(CFLAGS) -c Util/client_info_queue.c

common.o: Util/common.c
	$(CC) $(CFLAGS) -c Util/common.c

synch.o: Sync/synch.c
	$(CC) $(CFLAGS) -c Sync/synch.c

clean:
	rm -f main_c main_s $(OBJS)
