GCC = gcc
WALL = -Wall
DEBUG_FLAG = -ggdb
LEAK_FLAG = --leak-check=full
LOG_FLAG = --log-file
LIBS = -lpthread -lm 
VALGRIND = valgrind

SERV_PORT = 6000
DEST_IP = 192.168.2.202 
DEST_PORT = 5050

TARGET = proxy_rx
TARGET_ARGS = proxy_rx_args
DBG_TARGET = debug_proxy_rx

SRC = boss_server.c
DEP = forward.c pqueue.c link_receptor.c ../network/network.c 
LOG = rx_proxy.log

default: all

all: $(TARGET)

$(TARGET): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET) $(LIBS) -lconfig -DCONF_ENABLE -DRX_PROXY

$(TARGET_ARGS): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET_ARGS) $(LIBS) -DRX_PROXY 

$(DBG_TARGET): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(DBG_TARGET) $(DEBUG_FLAG) $(LIBS) $(WALL) -DRX_PROXY

clean:
	rm -rf $(TARGET_ARGS) $(TARGET) $(DBG_TARGET) $(LOG)

