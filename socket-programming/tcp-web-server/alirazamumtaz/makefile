all : myserver
myserver : main.o tcpwebserver.o
	gcc main.o tcpwebserver.o -o myserver
main.o : src/main.c
	gcc -c src/main.c
tcpwebserver.o : src/tcpwebserver.c
	gcc -c src/tcpwebserver.c
