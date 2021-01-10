CC = gcc
CFLAGS = -Wall -Os
TARGET = prxencrypter
OBJS = crypto.o kirk_engine.o main.o
LDFLAGS=-lz

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

clean:
	$(RM) *.o $(TARGET) *.exe *.exe.stackdump
