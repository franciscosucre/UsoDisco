all: usoDiscoMain.o messages.o
	gcc usoDiscoMain.o messages.o -lpthread -o UsoDisco

usoDiscoMain.o:
	gcc -g -c usoDiscoMain.c messages.c messages.h -lpthread
	
messages.o:
	gcc -g -c messages.c messages.h
	
clean:
	rm usoDiscoMain.o messages.o UsoDisco
