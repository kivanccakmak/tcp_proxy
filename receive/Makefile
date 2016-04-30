GCC = gcc

TARGET = receive
# to run with config file

TARGET_ARGS = receive_args
# to run with argument variables

SRC = receive.c

CFLAGS = -DRECV

OUT = output.txt

default: all

all: $(TARGET)

$(TARGET): $(SRC) 
	$(GCC) $(SRC) -o $(TARGET) -lconfig $(CFLAGS) -DCONF_ENABLE 

$(TARGET_ARGS): $(SRC)
	$(GCC) $(SRC) -o $(TARGET_ARGS) $(CFLAGS)

clean:
	rm -rf $(TARGET) $(TARGET_ARGS) $(OUT) $(OUT)
