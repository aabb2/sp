objects = s.o log/log.o init-server.o connect-to.o get-ip.o fds.o trans.o

all: a.out

a.out: ${objects}
	gcc -g -o a.out ${objects}
	
s.o: s.c sp.h
	gcc -g -c s.c
	
log/log.o: log/log.c log/log.h
	gcc -g -c log/log.c -o log/log.o
	
init-server.o: sp.h init-server.c
	gcc -g -c init-server.c

connect-to.o: connect-to.c sp.h
	gcc -g -c connect-to.c
	
get-ip.o: get-ip.c sp.h
	gcc -g -c get-ip.c
	
fds.o: fds/fds.h fds/fds.c
	gcc -g -c fds/fds.c
	
trans.o: trans.c
	gcc -g -c trans.c
	
.PHONY : clean
clean: 
	-rm -f *.o a.out ${objects}