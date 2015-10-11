server: server.c
	gcc server.c -o debug_server -ggdb 
	valgrind --log-file=marul.log --leak-check=full ./debug_server
client: client.c file_read.c
	gcc client.c file_read.c -o debug_client -ggdb
	valgrind --log-file=patates.log --leak-check=full ./debug_client 192.168.3.195 8888 file_list/kivanc.txt
file_try: file_read.c
	gcc file_read.c -o debug_read -ggdb
	valgrind --log-file=domates.log --leak-check=full ./debug_read file_list/kivanc.txt 0 10 
