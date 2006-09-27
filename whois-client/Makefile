PREFIX ?= /usr/
CC = gcc
ORBIT_IDL=$(PREFIX)/bin/orbit-idl-2
CFLAGS=$(shell pkg-config ORBit-2.0 ORBit-CosNaming-2.0 --cflags)
LDFLAGS=$(shell pkg-config ORBit-2.0 ORBit-CosNaming-2.0 --libs)

TARGETS=whois-client epp-client
IDLOUT=ccReg-common.c  ccReg-skels.c  ccReg-stubs.c  ccReg.h
all: $(IDLOUT) whois-client epp-client


whois-client : whois-client.o ccReg-common.o  ccReg-skels.o  ccReg-stubs.o examples-toolkit.o
epp-client : epp-client.o ccReg-common.o  ccReg-skels.o  ccReg-stubs.o examples-toolkit.o utf8hack.o

$(IDLOUT): ../idl/ccReg.idl
	$(ORBIT_IDL) ../idl/ccReg.idl

%-skelimpl.c: %.idl
	$(ORBIT_IDL) --skeleton-impl $^

clean:
	rm -rf *.o *~ $(IDLOUT) *.ior *.ref whois-client epp-client

