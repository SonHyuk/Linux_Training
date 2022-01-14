main_tar : tar_function.o main_tar.o
	gcc -o main_tar tar_function.o main_tar.o -lz

tar_function.o : tar_function.c
	gcc -c -o tar_function.o tar_function.c

main_tar.o : main_tar.c
	gcc -c -o main_tar.o main_tar.c


clean :
	rm *.o main_tar
