GCC = gcc
TARGET_ARGS = stream_args
TARGET = stream
SRC = stream.c
DEP = ../file_read/file_read.c

DEST_IP = 127.0.0.1 
DEST_PORT = 5050
FILENAME = book.txt

default: all

all: $(TARGET)

$(TARGET): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET) -lconfig -DSTREAM -DCONF_ENABLE

$(TARGET_ARGS): $(SRC) $(DEP) 
	$(GCC) $(SRC) $(DEP) -o $(TARGET) -DSTREAM
	./$(TARGET) $(DEST_IP) $(DEST_PORT) $(FILENAME)
clean:
	rm -f $(TARGET) $(TARGET_ARGS)
