CC = gcc
CFLAGS = -Wall -Wextra -g

# Link with the real-time library
LDFLAGS = -pthread -lrt

# Source files
SRCS = Client/main_client.c Server/main_server.c Util/client_info_queue.c Util/common.c Sync/synch.c Server/command_operation.c Server/server_util.c Client/client_command_operations.c

# Object files
OBJS = $(SRCS:.c=.o)

all: main_client main_server

main_client: Client/main_client.o Util/client_info_queue.o Util/common.o Sync/synch.o Client/client_command_operations.o
	$(CC) $(CFLAGS) -o main_c Client/main_client.o Util/client_info_queue.o Util/common.o Sync/synch.o Client/client_command_operations.o $(LDFLAGS)

main_server: Server/main_server.o Util/client_info_queue.o Util/common.o Sync/synch.o Server/command_operation.o Server/server_util.o
	$(CC) $(CFLAGS) -o main_s Server/main_server.o Util/client_info_queue.o Util/common.o Sync/synch.o Server/command_operation.o Server/server_util.o $(LDFLAGS)

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

command_operation.o: Server/command_operation.c
	$(CC) $(CFLAGS) -c Server/command_operation.c

server_util.o: Server/server_util.c
	$(CC) $(CFLAGS) -c Server/server_util.c

client_command_operations.o: Client/client_command_operations.c
	$(CC) $(CFLAGS) -c Client/client_command_operations.c

clean:
	rm -f main_c main_s $(OBJS)
