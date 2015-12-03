GCC = gcc
WALL = -Wall
DEBUG_FLAG = -ggdb
LEAK_FLAG = --leak-check=full
LOG_FLAG = --log-file
VALGRIND = valgrind
SPLIT_LOG = split.log

SPLIT_LIBRARIES = -lnetfilter_queue -lnfnetlink -lpthread
SPLIT_SOURCES = src/split.c 
SPLIT_TARGET = split

$(SPLIT_TARGET): $(SPLIT_SOURCES) 
	$(GCC) $(SPLIT_SOURCES) -o $(SPLIT_TARGET) $(DEBUG_FLAG) $(SPLIT_LIBRARIES) $(WALL)
	$(VALGRIND) $(LOG_FLAG)=$(SPLIT_LOG) $(LEAK_FLAG) ./$(SPLIT_TARGET) 
