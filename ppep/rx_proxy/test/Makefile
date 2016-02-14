GCC = gcc
DEBUG_FLAG = -ggdb
WALL = -Wall
LEAK_FLAG = --leak-check=full
LOG_FLAG = --log-file
VALGRIND = valgrind
LIBS = -lpthread -lm

DEST_IP = 127.0.0.1
DEST_PORT = 5050
PROXY_PORT = 6060
OUT_FILE = output.txt

TEST_LOG = rx_proxy.log

SRC = test.c
DEP = ../boss_server.c ../forward.c ../../receive/receive.c \
	  ../pqueue.c ../link_receptor.c ../../network/network.c

TARGET = rx_proxy_test
RUN = $(TARGET) $(DEST_PORT) $(DEST_IP) $(PROXY_PORT) $(OUT_FILE) 

$(TARGET): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET) $(DEBUG_FLAG) $(LIBS) $(WALL) -DTEST_RX_PROXY 
	$(VALGRIND) $(LOG_FLAG)=$(TEST_LOG) $(LEAK_FLAG) ./$(RUN)

