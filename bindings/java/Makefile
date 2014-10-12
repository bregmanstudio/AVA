CFLAGS += -shared -fPIC -g -I$(JDK)/include -I$(JDK)/include/linux -I/usr/local/include/

.SUFFIXES: .java .class .o

JDK=/usr/lib/jvm/java-6-openjdk

OBJS = libAudioDB_JNI.o
CLASSES = AudioDB.class
NATIVE_LIB = libAudioDB_JNI.so

build: $(CLASSES) $(NATIVE_LIB)

.java.class:
	$(JDK)/bin/javac $<

.class.h:
	$(JDK)/bin/javah -jni $(<:%.class=%)

$(NATIVE_LIB): $(OBJS)
	ld -fPIC -G $(OBJS) -laudioDB -o $@

