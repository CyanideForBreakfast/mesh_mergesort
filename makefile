all:
	gcc -o node queue.c node.c -lrt -lpthread
	gcc coordinator.c queue.c -lrt -lpthread

clean: 
	rm a.out
	rm node