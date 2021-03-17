all: thread

thread: *.c
	gcc -Wall -o schedule *.c  -lpthread
clean:
	rm schedule