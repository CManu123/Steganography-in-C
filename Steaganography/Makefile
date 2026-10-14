CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = lsb_steg
SRCS = main.c encode.c decode.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
.
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
