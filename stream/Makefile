GCC = gcc

TARGET = stream

SRC = stream.c

DEP += file_read/file_read.c              \
       ../commons/argv_reader/argv_read.c \

CFLAGS = -DSTREAM

default: all

all: $(TARGET)

$(TARGET): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET) -lconfig $(CFLAGS)

clean:
	rm -f $(TARGET)
