GCC = gcc
WALL = -Wall
DEBUG_FLAG = -ggdb
LEAK_FLAG = --leak-check=full
LOG_FLAG = --log-file
VALGRIND = valgrind
SPLIT_LOG = tx_proxy.log

LOCAL_PORT = 5000
RX_PROXY_IP = 192.168.2.12 
RX_PROXY_PORT = 6000

TARGET = proxy_tx 
DBG_TARGET = debug_proxy_tx
SRC = split.c 
DEP = ../network/network.c
LIBS = -lnetfilter_queue -lnfnetlink -lpthread

default: all

all: $(TARGET)

$(TARGET): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET) $(LIBS) -lconfig -DTX_PROXY -DCONF_ENABLE
$(TARGET_ARGS): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET_ARGS) $(LIBS) -DTX_PROXY
clean:
	rm -rf $(TARGET) $(TARGET_ARGS)
