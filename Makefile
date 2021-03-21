all: thread

thread: *.c
	gcc -Wall -o schedule *.c  -lpthread -lm
clean:
	rm schedule