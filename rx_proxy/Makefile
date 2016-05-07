GCC = gcc

TARGET = proxy_rx 
# to run with config file

SRC = boss_server.c

DEP += forward.c \
       link_receptor.c \
       libpqueue/pqueue.c \
       ../commons/network/network.c \
	   ../commons/argv_reader/argv_read.c \

LIBS += -lpthread \
        -lm \

CFLAGS = -DRX_PROXY

LOG = rx_proxy.log

default: all

all: $(TARGET)

$(TARGET): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET) $(LIBS) -lconfig $(CFLAGS)

clean:
	rm -rf $(TARGET) $(LOG)

