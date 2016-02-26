GCC = gcc
WALL = -Wall
DEBUG_FLAG = -ggdb
LEAK_FLAG = --leak-check=full
LOG_FLAG = --log-file
LIBS = -lpthread -lm 
VALGRIND = valgrind

SERV_PORT = 6000
DEST_IP = 127.0.0.1 
DEST_PORT = 5050

TARGET = proxy_rx
SRC = boss_server.c
DEP = forward.c pqueue.c link_receptor.c ../network/network.c 
LOG = rx_proxy.log

RUN = $(TARGET) $(SERV_PORT) $(DEST_IP) $(DEST_PORT)

$(TARGET): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET) $(LIBS) -DRX_PROXY 
	./$(RUN) 
clean:
	rm -rf $(TARGET) $(DBG_TARGET) $(LOG)

