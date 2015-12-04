GCC = gcc
TARGET = stream
SRC = stream.c
DEP = ../relay/file_read.c

$(TARGET): $(SRC) $(DEP) 
	$(GCC) $(SRC) $(DEP) -o $(TARGET)
clean:
	rm -f $(TARGET)
