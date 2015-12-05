GCC = gcc
TARGET = stream
SRC = stream.c
DEP = ../relay/file_read.c

DEST_IP = 192.168.2.22
DEST_PORT = 5000
FILENAME = kivanc.txt

$(TARGET): $(SRC) $(DEP) 
	$(GCC) $(SRC) $(DEP) -o $(TARGET)
	./$(TARGET) $(DEST_IP) $(DEST_PORT) $(FILENAME)
clean:
	rm -f $(TARGET)
