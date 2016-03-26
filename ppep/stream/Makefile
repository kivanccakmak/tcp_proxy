GCC = gcc
TARGET = stream
SRC = stream.c
DEP = ../file_read/file_read.c

DEST_IP = 192.168.2.201 
DEST_PORT = 5050
FILENAME = book.txt

$(TARGET): $(SRC) $(DEP) 
	$(GCC) $(SRC) $(DEP) -o $(TARGET) -DSTREAM
	./$(TARGET) $(DEST_IP) $(DEST_PORT) $(FILENAME)
clean:
	rm -f $(TARGET)
