GCC = gcc
TARGET = stream
SRC = stream.c
DEP = ../file_read/file_read.c

DEST_IP = 192.168.3.195 
DEST_PORT = 5050
FILENAME = book.txt

$(TARGET): $(SRC) $(DEP) 
	$(GCC) $(SRC) $(DEP) -o $(TARGET) -DSTREAM
	./$(TARGET) $(DEST_IP) $(DEST_PORT) $(FILENAME)
clean:
	rm -f $(TARGET)
