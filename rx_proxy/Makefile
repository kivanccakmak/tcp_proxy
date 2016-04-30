GCC = gcc

TARGET = proxy_rx 
# to run with config file
#
TARGET_ARGS = proxy_rx_args  
# to run with argument variables

SRC = boss_server.c

DEP += forward.c \
       link_receptor.c \
       libpqueue/pqueue.c \
       ../commons/network/network.c

LIBS += -lpthread \
        -lm 

CFLAGS = -DRX_PROXY

LOG = rx_proxy.log

default: all

all: $(TARGET)

$(TARGET): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET) $(LIBS) -lconfig $(CFLAGS) -DCONF_ENABLE

$(TARGET_ARGS): $(SRC) $(DEP)
	$(GCC) $(SRC) $(DEP) -o $(TARGET_ARGS) $(LIBS) $(CFLAGS) 

clean:
	rm -rf $(TARGET_ARGS) $(TARGET) $(DBG_TARGET) $(LOG)

