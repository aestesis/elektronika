# Makefile for Linux Open Video Plugins

all-am: FreeFrame.so


FreeFrameSample.so: FreeFrame.cpp FreeFrameSample.cpp
	gcc -c -O3 -DLINUX -mmmx FreeFrame.cpp -o FreeFrame.o 
	g++ -shared -O3 -DLINUX -mmmx FreeFrameSample.cpp FreeFrame.o -o FreeFrameSample.so -I../../MSVCPlugin/
clean:
	rm -f *.so
