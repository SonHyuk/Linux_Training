main_tar : main_tar.o
	gcc -o main_tar main_tar.o -lz

main.o : main_tar.c
	gcc -c -o main_tar.o main_tar.c

clean :
	rm *.o main_tar
