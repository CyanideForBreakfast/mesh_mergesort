all:
	gcc -o node node.c -lrt -lpthread
	gcc coordinator.c -lrt -lpthread

clean: 
	rm a.out
	rm node