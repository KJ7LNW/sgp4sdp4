#Makefile for ephem     March 30 2001

SHELL = /bin/sh
CC    = gcc -Wall -O2 -march=native -D_FORTIFY_CODE=2

objects = main.o sgp4sdp4.o sgp_in.o sgp_math.o sgp_obs.o sgp_time.o solar.o

kelso : $(objects)
	$(CC) -lm -o ephem $(objects)

$(objects) : sgp4sdp4.h

.PHONY : clean
clean  :
	-rm -f *.o *~

distclean : clean
	rm -f $(PROJECT)
