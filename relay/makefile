server: server.c
	gcc server.c -o debug_server -ggdb 
client: client.c
	gcc client.c -o debug_client -ggdb
	valgrind --log-file=patates.log --leak-check=full ./debug_client 192.168.3.195 8888
