GCC = gcc
TARGET = stream
SRC = stream.c
DEP = ../file_read/file_read.c

DEST_IP = 10.20.0.228 
DEST_PORT = 5000
FILENAME = kivanc.txt

$(TARGET): $(SRC) $(DEP) 
	$(GCC) $(SRC) $(DEP) -o $(TARGET) -DSTREAM
	./$(TARGET) $(DEST_IP) $(DEST_PORT) $(FILENAME)
clean:
	rm -f $(TARGET)
