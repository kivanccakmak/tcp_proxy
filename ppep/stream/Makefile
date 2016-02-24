GCC = gcc
TARGET = stream
SRC = stream.c
DEP = ../file_read/file_read.c

DEST_IP = 127.0.0.1 
DEST_PORT = 5050
FILENAME = kivanc.txt

$(TARGET): $(SRC) $(DEP) 
	$(GCC) $(SRC) $(DEP) -o $(TARGET) -DSTREAM
	./$(TARGET) $(DEST_IP) $(DEST_PORT) $(FILENAME)
clean:
	rm -f $(TARGET)
