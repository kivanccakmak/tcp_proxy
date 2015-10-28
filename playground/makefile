clean:
	rm debug_some patates.log
some: play_some.c
	gcc play_some.c -o debug_some -ggdb
	valgrind --log-file=domates.log --leak-check=full ./debug_some www.google.com
