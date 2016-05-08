GCC = gcc

TARGET = proxy_tx

SRC = split.c 

DEP += ../commons/network/network.c \
	   ../commons/argv_reader/argv_read.c \

LIBS += -lnetfilter_queue \
        -lnfnetlink \
        -lpthread \

CFLAGS = -DTX_PROXY

default: all

all: $(TARGET)

$(TARGET): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET) $(LIBS) -lconfig $(CFLAGS)

clean:
	rm -rf $(TARGET)
