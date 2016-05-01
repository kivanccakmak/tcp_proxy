GCC = gcc

TARGET = receive
# to run with config file

TARGET_ARGS = receive_args
# to run with argument variables

SRC = receive.c

DEP += ../commons/argv_reader/argv_read.c

CFLAGS = -DRECV

OUT = output.txt

default: all

all: $(TARGET)

$(TARGET): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET) -lconfig $(CFLAGS) -DCONF_ENABLE

$(TARGET_ARGS): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET_ARGS) $(CFLAGS) -DARGV_ENABLE

clean:
	rm -rf $(TARGET) $(TARGET_ARGS) $(OUT) $(OUT)
