objects = main.o client.o serverFunctions.o ServerThreadHandles.o SigHandlers.o
CFLAGS	= -I . -g -ggdb
headers = serverHeader.h client.h ServerThreadHandles.h SigHandlers.h ../msgdist_defaults.h ../text.h ../topic.h ../typedefines.h

server: $(objects)
	gcc -o server $(objects) $(CFLAGS) -pthread

main.o:	main.c $(headers)

client.o:	client.c $(headers)

serverFunctions.o:	serverFunctions.c $(headers)

ServerThreadHandles.o:	ServerThreadHandles.c $(headers)

SigHandles.o:	SigHandles.c $(headers)

verifier:	verificador.o
	gcc -o verificador verificador.c $(CFLAGS)

clean:
	rm ./*.o && rm ./server && rm ./verificador
