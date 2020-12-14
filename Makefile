CC = gcc -Wall

main : initial archiviste journaliste

initial : initial.c types.h

archiviste : archiviste.c types.h

journaliste : journaliste.c types.

clean :
	rm -f initial archiviste journaliste