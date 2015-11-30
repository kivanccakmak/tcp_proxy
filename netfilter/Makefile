GCC = gcc
DEBUG_FLAG = -ggdb
LEAK_FLAG = --leak-check=full
LOG_FLAG = --log-file
LOGFILE = atom.log

LIBRARIES = -lnetfilter_queue -lnfnetlink -lpthread

SOURCES = $(wildcard src/*.c)
HEADERS = $(wildcard src/.h)

TARGET = atom

$(TARGET): $(SOURCES) $(HEADERS)
	$(GCC) $(SOURCES) -o $(TARGET) $(DEBUG_FLAG) $(LIBRARIES)
	$(VALGRIND) $(LOG_FLAG)=$(LOGFILE) $(LEAK_FLAG) ./$(TARGET) 
