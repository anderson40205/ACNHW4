all:main arp
	gcc arp.o main.o -o arp
main:main.o
	gcc -c main.c -o main.o
arp:arp.o
	gcc -c arp.c -o arp.o
clean:
	rm *.o arp