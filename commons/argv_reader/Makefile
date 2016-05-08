GCC = gcc

DBG_FLAG += -g    \
            -Wall \

SRC = argv_read.c

TARGET = argv_read

default: all

all: $(SRC)
	$(GCC) $(SRC) -o $(TARGET) -DARGV_READ

clean:
	rm -rf $(TARGET)

