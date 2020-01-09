objects = main.o Functions.o PipeFunctions.o SigHandlers.o ThreadHandlers.o
CFLAGS	= -I . -g -ggdb
headers = clientHeader.h Functions.h PipeFunctions.h SigHandlers.h ThreadHandlers.h ../msgdist_defaults.h ../text.h ../topic.h ../typedefines.h

client:	$(objects)
	gcc -o client $(objects) $(CFLAGS) -pthread -lncurses

main.o:	main.c $(headers)

Functions.o:	Functions.c $(headers)

PipeFunctions.o:	PipeFunctions.c $(headers)

SigHandlers.o:	SigHandlers.c $(headers)

ThreadHandlers.o:	ThreadHandlers.c $(headers)

clean:
	rm	./*.o && rm ./client
