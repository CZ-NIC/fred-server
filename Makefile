
CC=gcc
CXX = g++

CXXFLAGS = -O2  -Wall -DDEBUG
OBJECTS = 
LDFLAGS=  -L/usr/local/pgsql/lib/
LIBS=  -lomniORB4 -lomniDynamic4 -lomnithread -lpthread
CPPFLAGS =  -I/usr/local/pgsql/include/ -I.  -Wno-deprecated
SERVER_OBJECTS=ccRegSK.o  ccReg_epp.o  ccReg_server.o   pqsql.o
CLIENT_OBJECTS=ccRegSK.o epp_client.o
all: server client
.SUFFIXES:  .o

server: $(SERVER_OBJECTS)
	$(CXX) -o server $(SERVER_OBJECTS) $(LDFLAGS) $(LIBS) -lpq


client: $(CLIENT_OBJECTS)
	$(CXX) -o client $(CLIENT_OBJECTS) $(LDFLAGS) $(LIBS)



%.o: %.cc
	$(CXX) $(CPPFLAGS) -c $<



clean:
	rm -rf *.o server client


