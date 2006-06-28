CPPUNITLIBS=$(shell cppunit-config --libs)
SOURCES=main.cc cr.cc 

all: test

test: $(SOURCES)
	g++ -I ..  -ggdb -o $@ $(SOURCES)  ../libccreg.a ${CPPUNITLIBS} -lomniORB4 -lomniDynamic4 -lomnithread -lpq