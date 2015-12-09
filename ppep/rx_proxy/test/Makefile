GCC = gcc
DEBUG_FLAG = -ggdb 
WALL= -Wall
LEAK_FLAG = --leak-check=full
LOG_FLAG = --log-file
VALGRIND = valgrind
LIBS = -lpthread -lm

DEST_IP = 127.0.0.1
DEST_PORT = 5050
TEST_SRC = test.c
OUT_FILE = output.txt

# to test queue module
QUEUE_DEP = ../queue.c ../reorder.c ../link_receptor.c \
			../../network/network.c
QUEUE_TARGET = queue
QUEUE_LOG = queue.log
QUEUE_RUN = $(QUEUE_TARGET) $(DEST_IP) $(DEST_PORT) $(OUT_FILE)

REORDER_TARGET = reorder
REORDER_DEP = ../queue.c ../reorder.c ../link_receptor.c
REORDER_LOG = reorder.log
REORDER_RUN = $(REORDER_TARGET) $(DEST_IP) $(DEST_PORT) $(OUT_FILE)

# to run receiver daemon
RECV_TARGET = recv
RECV_LOG = recv.log
RECV_RUN = $(RECV_TARGET) $(DEST_IP) $(DEST_PORT) $(OUT_FILE) 



$(QUEUE_TARGET): $(TEST_SRC) $(QUEUE_DEP)
	$(GCC) $(TEST_SRC) $(QUEUE_DEP) -o $(QUEUE_TARGET) $(DEBUG_FLAG) $(LIBS) $(WALL) -DTEST_QUEUE
	$(VALGRIND) $(LOG_FLAG)=$(QUEUE_LOG) $(LEAK_FLAG) ./$(QUEUE_RUN) 
$(REORDER_TARGET): $(TEST_SRC) $(REORDER_DEP)
	$(GCC) $(TEST_SRC) $(REORDER_DEP) -o $(REORDER_TARGET) $(DEBUG_FLAG) $(LIBS) $(WALL) -DTEST_REORDER 
	$(VALGRIND) $(LOG_FLAG)=$(REORDER_LOG) $(LEAK_FLAG) ./$(REORDER_RUN)
$(RECV_TARGET): $(TEST_SRC)  
	$(GCC) $(TEST_SRC) -o $(RECV_TARGET) $(DEBUG_FLAG) $(THREAD_FLAG) $(WALL) $(LIBS) -DTEST_RECV 
	$(VALGRIND) $(LOG_FLAG)=$(RECV_LOG) $(LEAK_FLAG) ./$(RECV_RUN)
clean:
	rm -rf $(OUT_FILE) $(QUEUE_TARGET) $(RECV_TARGET) 
