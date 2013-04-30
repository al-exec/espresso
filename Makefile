espresso:	espresso_pthread.c main.c espresso_sync.c espresso_jpeg.c fuzz_function.c
	gcc -ggdb -ljpeg -lpthread -Wall -o espresso espresso_pthread.c espresso_jpeg.c main.c fuzz_function.c espresso_sync.c -I.

