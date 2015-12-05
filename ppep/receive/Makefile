GCC = gcc

PORT = 5000

RECV_TARGET = receive
RECV_SRC = receive.c
OUTPUT = output.txt

$(RECV_TARGET): $(RECV_SRC)
	$(GCC) $(RECV_SRC) -o $(RECV_TARGET)
	./$(RECV_TARGET) $(PORT) $(OUTPUT)
clean:
	rm -rf $(RECV_TARGET) $(OUTPUT)

