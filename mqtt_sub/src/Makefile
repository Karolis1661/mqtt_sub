CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lmosquitto -lsqlite3 -luci -ljson-c -lcurl

TARGET = mqtt_sub
DEPS = $(wildcard src/*.h)
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

.PHONY: clean

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(OBJS) -f $(TARGET)