CC=gcc
CFLAGS=-Wall -Werror -Wformat -Wpedantic
TARGET=a.out
OBJECTS=src/main.o

all: $(TARGET)
	 #./a.out

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm $(OBJECTS) $(TARGET)
