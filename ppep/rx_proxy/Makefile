GCC = gcc
WALL = -Wall
DEBUG_FLAG = -ggdb
LEAK_FLAG = --leak-check=full
LOG_FLAG = --log-file
LIBS = -lpthread -lm 
VALGRIND = valgrind

SERV_PORT = 5000
DEST_IP = 192.168.2.202 
DEST_PORT = 5050

TARGET = proxy_rx
DBG_TARGET = debug_proxy_rx
DEP = reorder.c link_receptor.c queue.c ../network/network.c 
SRC = boss_server.c
LOG = rx_proxy.log

RUN = $(TARGET) $(SERV_PORT) $(DEST_IP) $(DEST_PORT)
DBG_RUN = $(DBG_TARGET) $(SERV_PORT) $(DEST_IP) $(DEST_PORT) 

$(TARGET): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET) $(LIBS) -DRX_PROXY 
	./$(RUN) 
$(DBG_TARGET): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(DBG_TARGET) $(DEBUG_FLAG) $(LIBS) $(WALL) -DRX_PROXY
	$(VALGRIND) $(LOG_FLAG)=$(LOG) $(LEAK_FLAG) ./$(DBG_RUN)  
clean:
	rm -rf $(TARGET) $(DBG_TARGET) $(LOG)

