SBCL=sbcl

all:
	$(SBCL) --eval '(require :asdf)' --eval '(require :sb-adb)' --eval '(quit)'

clean:
	-rm *.fasl
	-rm testdb*

test: ../../libaudioDB.so.0.0
	$(SBCL) --disable-debugger --eval '(require :asdf)' --eval '(require :sb-adb)'  --load tests.lisp --eval '(sb-adb::run-tests)' --eval '(quit)'
