GCC = gcc

TARGET = receive

SRC = receive.c

DEP += ../commons/argv_reader/argv_read.c

CFLAGS = -DRECV

OUT = output.txt

default: all

all: $(TARGET)

$(TARGET): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET) -lconfig $(CFLAGS)

clean:
	rm -rf $(TARGET) $(OUT) $(OUT)
