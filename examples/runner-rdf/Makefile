all: populate

populate: populate.c
	gcc -o populate -std=c99 -D_GNU_SOURCE -lrdf -laudioDB -L../.. -I../.. -Wl,-rpath $(shell pwd)/../.. $<

clean:
	-rm populate keyplot.adb
