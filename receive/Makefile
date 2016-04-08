GCC = gcc

PORT = 5050 

RECV_TARGET = receive
RECV_SRC = receive.c
OUTPUT = output.txt

LOG_FILE = out.log

$(RECV_TARGET): $(RECV_SRC)
	$(GCC) $(RECV_SRC) -o $(RECV_TARGET) -DRECV
	./$(RECV_TARGET) $(PORT) $(OUTPUT) $(LOG_FILE)
clean:
	rm -rf $(RECV_TARGET) $(OUTPUT) $(LOG_FILE)

