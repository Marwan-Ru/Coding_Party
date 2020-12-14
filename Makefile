CC = gcc -Wall

main : clean initial archiviste journaliste

initial : initial.c types.h

archiviste : archiviste.c types.h

journaliste : journaliste.c types.h

clean :
	rm -f initial archiviste journaliste