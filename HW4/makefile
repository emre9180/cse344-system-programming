CC = gcc-13
CFLAGS = -Wall -Wextra

TARGET = main
SRCS = main.c threads.c
OBJS = $(addprefix OBJECT_F/,$(SRCS:.c=.o))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

OBJECT_F/%.o: %.c | OBJECT_F
	$(CC) $(CFLAGS) -c $< -o $@

OBJECT_F:
	mkdir -p OBJECT_F

clean:
	rm -f $(OBJS) $(TARGET)
