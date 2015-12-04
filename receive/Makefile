GCC = gcc

PORT = 5000

RECV_TARGET = receive
RECV_SRC = receive.c

$(RECV_TARGET): $(RECV_SRC)
	$(GCC) $(RECV_SRC) -o $(RECV_TARGET)
	./$(RECV_TARGET) $(PORT)
clean:
	rm -rf $(RECV_TARGET)

