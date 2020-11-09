all:
	gcc -o node common.c node.c -lrt -lpthread
	gcc coordinator.c common.c -lrt -lpthread

clean: 
	rm a.out
	rm node