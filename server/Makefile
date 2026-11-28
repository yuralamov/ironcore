CC = clang
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -O2 -I../common
LDFLAGS = $(shell pkg-config --libs libenet)
TARGET = ironcore_server
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
