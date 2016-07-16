CC = arm-fsl-linux-gnueabi-gcc

ks103: ks103-test.o
	$(CC) -o ks103 ks103-test.o

ks103-test.o: ks103-test.c
	$(CC) -c ks103-test.c

clean:
	rm -vf ks103-test.o ks103

.PHONY: clean
