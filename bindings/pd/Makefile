LIBS = -L/usr/local/lib/ -laudioDB
CXX = gcc

adbpd.pd_linux: adbpd.o 
	$(CXX) -o $@ -shared -Wl,-export-dynamic $(LDFLAGS) $^ $(LIBS)

adbpd.o: adbpd.c
	$(CXX) -c -fPIC -I/usr/local/include/ $< -o $@

clean:
	rm -f adbpd.o
	rm -f adbpd.pd_linux

test:
