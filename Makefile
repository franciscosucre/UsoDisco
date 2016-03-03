all: usoDiscoMain.o messages.o
	gcc usoDiscoMain.o messages.o directoryStack.o pthreadQueue.o -o UsoDisco

usoDiscoMain.o:
	gcc -g -c usoDiscoMain.c messages.c messages.h directoryStack.c directoryStack.h pthreadQueue.c pthreadQueue.h
	
messages.o:
	gcc -g -c messages.c messages.h
	
directoryStack.o:
	gcc -g -c directoryStack.c directoryStack.h
	
pthreadQueue.o:
	gcc -g -c pthreadQueue.c pthreadQueue.h

clean:
	rm usoDiscoMain.o messages.o directoryStack.o pthreadQueue.o UsoDisco
