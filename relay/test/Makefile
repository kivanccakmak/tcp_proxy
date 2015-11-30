GCC = gcc
DEBUG_FLAG = -ggdb 
WALL= -Wall
LEAK_FLAG = --leak-check=full
LOG_FLAG = --log-file
THREAD_FLAG = -lpthread

VALGRIND = valgrind
TEST_INPUT = ../file_list/kivanc.txt

DEST_IP = 127.0.0.1 
DEST_PORT = 5050

TEST_SRC = test.c

# to test queue module
QUEUE_DEP = ../queue.c ../reorder.c ../link_receptor.c 
QUEUE_FLAG = -que
QUEUE_TARGET = test_queue
QUEUE_LOG = queue.log
QUEUE_RUN = $(QUEUE_TARGET) $(QUEUE_FLAG) $(DEST_IP) $(DEST_PORT)

# to test reorder module
REORDER_DEP = ../queue.c ../reorder.c ../link_receptor.c 
REORDER_TARGET = test_reorder
REORDER_FLAG = -reord
REORDER_LOG = reorder.log

# to run receiver daemon 
RECV_TARGET = debug_recv
RECV_FLAG = -recv
RECV_LOG = recv.log

REORDER_RUN = $(REORDER_TARGET) $(REORDER_FLAG) $(DEST_IP) $(DEST_PORT)

$(QUEUE_TARGET): $(TEST_SRC) $(QUEUE_DEP)
	$(GCC) $(TEST_SRC) $(QUEUE_DEP) -o $(QUEUE_TARGET) $(DEBUG_FLAG) $(THREAD_FLAG) $(WALL) -lm
	$(VALGRIND) $(LOG_FLAG)=$(QUEUE_LOG) $(LEAK_FLAG) ./$(QUEUE_RUN) 
$(REORDER_TARGET): $(TEST_SRC) $(REORDER_DEP)
	$(GCC) $(TEST_SRC) $(REORDER_DEP) -o $(REORDER_TARGET) $(DEBUG_FLAG) $(THREAD_FLAG) $(WALL) -lm 
	$(VALGRIND) $(LOG_FLAG)=$(REORDER_LOG) $(LEAK_FLAG) ./$(REORDER_RUN)
$(RECV_TARGET): $(TEST_SRC)  
	$(GCC) $(TEST_SRC) -o $(RECV_TARGET) $(DEBUG_FLAG) $(THREAD_FLAG) $(WALL) -lm
	$(VALGRIND) $(LOG_FLAG)=$(RECV_LOG) $(LEAK_FLAG) ./$(RECV_TARGET)
clean:
	rm -rf $(QUEUE_TARGET) $(QUEUE_LOG)
