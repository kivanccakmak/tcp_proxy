GCC = gcc
WALL = -Wall
DEBUG_FLAG = -ggdb
LEAK_FLAG = --leak-check=full
LOG_FLAG = --log-file
VALGRIND = valgrind
SPLIT_LOG = split.log

LOCAL_PORT = 5000
DEST_IP = 192.168.2.202
DEST_PORT = 5050

SPLIT_TARGET = split
SPLIT_LIBS = -lnetfilter_queue -lnfnetlink -lpthread
SPLIT_SRC = src/split.c 
SPLIT_DEP = ../relay/network.c
SPLIT_DEP = ../relay/network.c
DEBUG_TARGET = debug_split
GDB_FLAG = -g

SPLIT_RUN = $(SPLIT_TARGET) $(LOCAL_PORT) $(DEST_IP) $(DEST_PORT)

$(SPLIT_TARGET): $(SPLIT_SRC) $(SPLIT_DEP) 
	$(GCC) $(SPLIT_SRC) $(SPLIT_DEP) -o $(SPLIT_TARGET) $(DEBUG_FLAG) $(SPLIT_LIBS) $(WALL)
	$(VALGRIND) $(LOG_FLAG)=$(SPLIT_LOG) $(LEAK_FLAG) ./$(SPLIT_RUN) 
$(DEBUG_TARGET): $(SPLIT_SOURCES) 
	$(GCC) $(GDB_FLAG) $(SPLIT_SOURCES) -o $(DEBUG_TARGET) $(SPLIT_LIBRARIES) 
