GCC = gcc
TARGET = receive
TARGET_ARGS = receive_args
SRC = receive.c

PORT = 5050 
OUT = output.txt
LOG = out.log

default: all

all: $(TARGET)

$(TARGET): $(SRC) 
	$(GCC) $(SRC) -o $(TARGET) -lconfig -DRECV -DCONF_ENABLE

$(TARGET_ARGS): $(SRC)
	$(GCC) $(SRC) -o $(TARGET_ARGS) -DRECV

clean:
	rm -rf $(TARGET) $(TARGET_ARGS) $(OUT) $(OUT)
