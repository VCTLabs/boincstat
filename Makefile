boincstat: boincstat.o window.o
	gcc `xml2-config --libs` -lncurses -o boincstat boincstat.o window.o 

# on Solaris, you may need
# CFLAGS=-DNO_GETOPT_LONG
boincstat.o: boincstat.c common.h
	gcc -c `xml2-config --cflags` ${CFLAGS}  boincstat.c

window.o: window.c common.h
	gcc -c window.c
