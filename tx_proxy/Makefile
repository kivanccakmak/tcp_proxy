GCC = gcc
TARGET = proxy_tx # to run with config file 
TARGET_ARGS = proxy_tx_args # to run with argument variables

SRC = split.c 
DEP = ../network/network.c
LIBS = -lnetfilter_queue -lnfnetlink -lpthread

CFLAGS = -DTX_PROXY

default: all

all: $(TARGET)

$(TARGET): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET) $(LIBS) -lconfig $(CFLAGS) -DCONF_ENABLE
$(TARGET_ARGS): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET_ARGS) $(LIBS) $(CFLAGS)
clean:
	rm -rf $(TARGET) $(TARGET_ARGS)
