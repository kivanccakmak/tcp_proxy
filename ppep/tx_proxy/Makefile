GCC = gcc
WALL = -Wall
DEBUG_FLAG = -ggdb
LEAK_FLAG = --leak-check=full
LOG_FLAG = --log-file
VALGRIND = valgrind
SPLIT_LOG = tx_proxy.log

LOCAL_PORT = 5000
RX_PROXY_IP = 127.0.0.1 
RX_PROXY_PORT = 6000

TARGET = proxy_tx 
DBG_TARGET = debug_proxy_tx
SRC = split.c 
DEP = ../network/network.c
LIBS = -lnetfilter_queue -lnfnetlink -lpthread

RUN = $(TARGET) $(LOCAL_PORT) $(RX_PROXY_IP) \
	  $(RX_PROXY_PORT)

DBG_RUN = $(DBG_TARGET) $(LOCAL_PORT) $(RX_PROXY_IP)\
		  $(RX_PROXY_PORT)

$(TARGET): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET) $(LIBS) -DTX_PROXY
	./$(RUN) 
$(DBG_TARGET): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(DBG_TARGET) $(DEBUG_FLAG) $(LIBS) $(WALL) -DTX_PROXY
	$(VALGRIND) $(LOG_FLAG)=$(SPLIT_LOG) $(LEAK_FLAG) ./$(DBG_RUN) 
clean:
	rm -rf $(TARGET) $(DBG_TARGET)
