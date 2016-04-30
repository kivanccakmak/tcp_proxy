GCC = gcc

TARGET = stream
# to run with config file

TARGET_ARGS = stream_args
# to run with argument variables

SRC = stream.c

DEP = file_read/file_read.c

CFLAGS = -DSTREAM

default: all

all: $(TARGET)

$(TARGET): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET) -lconfig $(CFLAGS) -DCONF_ENABLE

$(TARGET_ARGS): $(SRC) $(DEP) 
	$(GCC) $(SRC) $(DEP) -o $(TARGET) $(CFLAGS)
clean:
	rm -f $(TARGET) $(TARGET_ARGS)
