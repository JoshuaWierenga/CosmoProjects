TOOLCHAIN_PREFIX = $(shell realpath ../superconfigure)

APELINK ?= $(TOOLCHAIN_PREFIX)/cosmopolitan/bin/apelink
COSMOS ?= $(TOOLCHAIN_PREFIX)/cosmos/x86_64
MODE=

COSMOCC ?= $(TOOLCHAIN_PREFIX)/cosmopolitan/bin/x86_64-unknown-cosmo-cc
LINUXCC ?= gcc
WINCC ?= x86_64-w64-mingw32-gcc

PREFIX ?= build


.PHONY: build clean
build: libtestlib.glibc.so libtestlib.dll testprog.glibc.zip testprog.cosmo.zip testprog.cosmo2.com testprog.crt.zip

clean:
	rm -rf $(PREFIX) testprog.glibc.zip testprog.cosmo.zip testprog.crt.zip


$(PREFIX)/include/testlib.h: testlib.h
	mkdir -p $(PREFIX)/include/
	cp $< $(PREFIX)/include/

$(PREFIX)/lib/libtestlib.glibc.so: testlib.c $(PREFIX)/include/testlib.h
	mkdir -p $(PREFIX)/lib/
	$(LINUXCC) -shared -Wall -Werror -fpic -o $@ $<
libtestlib.glibc.so: $(PREFIX)/lib/libtestlib.glibc.so

$(PREFIX)/lib/libtestlib.dll: testlib.c $(PREFIX)/include/testlib.h
	mkdir -p $(PREFIX)/lib/
	$(WINCC) -shared -Wall -Werror -fpic -o $@ $<
libtestlib.dll: $(PREFIX)/lib/libtestlib.dll


$(PREFIX)/bin/testprog.glibc: testprog.c testlib.h
	mkdir -p $(PREFIX)/bin/
	$(LINUXCC) -Wall -o $@ $< -I$(PWD)
testprog.glibc: $(PREFIX)/bin/testprog.glibc

testprog.glibc.zip: $(PREFIX)/bin/testprog.glibc $(PREFIX)/lib/libtestlib.glibc.so
	zip -jq $@ $^


$(PREFIX)/bin/testprog.cosmo.com: testprog.c testlib.h
	mkdir -p $(PREFIX)/bin/
	$(COSMOCC) --update
	$(COSMOCC) -Wall -Werror -o $@ $< -I$(PWD)
	rm $@.dbg
testprog.cosmo.com: $(PREFIX)/bin/testprog.cosmo.com

testprog.cosmo.zip: $(PREFIX)/bin/testprog.cosmo.com $(PREFIX)/lib/libtestlib.glibc.so $(PREFIX)/lib/libtestlib.dll
	zip -jq $@ $^


$(PREFIX)/bin/testprog.cosmo2.com: testprog.c testlib.h $(PREFIX)/lib/libtestlib.glibc.so $(PREFIX)/lib/libtestlib.dll
	mkdir -p $(PREFIX)/bin/
	$(COSMOCC) --update
	$(COSMOCC) -Wall -Werror -o $@ $< -I$(PWD) -D _COSMO_SOURCE -D COSMO2=1
	zip -jq $@ $(filter-out $< $(word 2,$^),$^)
	rm $@.dbg
testprog.cosmo2.com: $(PREFIX)/bin/testprog.cosmo2.com


$(PREFIX)/bin/testprog.crt.exe: testprog.c testlib.h
	mkdir -p $(PREFIX)/bin/
	$(WINCC) -Wall -o $@ testprog.c -I$(PWD)
testprog.crt.exe: $(PREFIX)/bin/testprog.crt.exe

testprog.crt.zip: $(PREFIX)/bin/testprog.crt.exe $(PREFIX)/lib/libtestlib.dll
	zip -jq $@ $^
