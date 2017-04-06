LIBS=../obj/ccmalloc-$(CC).o -L../lib -lccmalloc -ldl

.SUFFIXES: .exe .c .cc

.cc.exe: $*.cc $(DEPS)
	$(CC) $(CFLAGS) -o $@ $*.cc $(LIBS)
.c.exe: $*.c ../obj/ccmalloc-$(CC).o ../lib/libccmalloc.a
	$(CC) $(CFLAGS) -o $@ $*.c $(LIBS)
.c.o: $*.c
	$(CC) $(CFLAGS) -c -o $@ $*.c
all:
	./testall
clean:
	rm -f *.exe *.log *.log.gz *.log.* .ccmalloc core a.out
	rm -f lib_test_C_11.a lib_test_C_19.so *.o

# always generate libraries
#
lib_test_C_11.a: # forced recompilation every time
	$(CC) $(CFLAGS) -c lib_test_C_11.c
	ar cr $@ lib_test_C_11.o
	ranlib $@

test_C_11.exe: lib_test_C_11.a test_C_11.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $*.c -L. -l_test_C_11 $(LIBS)

test_C_13.exe: test_C_13.c $(DEPS)
	$(CC) -o $@ -O $*.c $(LIBS)

# TODO generalize omit-frame-pointer flag to other compilers
#
test_C_14.exe: test_C_14.c $(DEPS)
	gcc -o $@ -fomit-frame-pointer -O2 $*.c $(LIBS)
	
# TODO generalize dynamic linking flags to other linkers than gcc
#
lib_test_C_19.so: # forced recompilation every time
	gcc -fPIC -c -o lib_test_C_19.o lib_test_C_19.c
	gcc -shared -WL,-soname,$@ -o $@ lib_test_C_19.o

test_C_19.exe: test_C_19.o lib_test_C_19.so $(DEPS)
	$(CC) $(CFLAGS) -o $@ test_C_19.o -L. -l_test_C_19 $(LIBS)
