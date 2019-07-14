CC=gcc
CFLAGS=-I../../include/ -I/opt/intel/sgxsdk/include -I../trusted/untrusted/ -I../lib
DEPS = spinlock.h hotcall-untrusted.h postfix_translator.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

hotcall.a: spinlock.o hotcall-bundler-untrusted.o postfix_translator.o ../lib/hotcall-hmap.o ../lib/hotcall-list.o ../lib/hotcall-hash.o
	ar rcsv libhotcall_bundler_untrusted.a spinlock.o hotcall-bundler-untrusted.o postfix_translator.o ../lib/hotcall-hmap.o ../lib/hotcall-list.o ../lib/hotcall-hash.o

clean:
	rm -rf *.o
