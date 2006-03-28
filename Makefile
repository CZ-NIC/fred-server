
CXX = g++

CXXFLAGS = -O2  -Wall -DDEBUG
OBJECTS = 
IDLFILE = ../idl/ccReg.idl
LDFLAGS=  -L/usr/local/pgsql/lib/
LIBS=  -lomniORB4 -lomniDynamic4 -lomnithread -lpthread
CPPFLAGS =  -I/usr/local/pgsql/include/ -I.  -Wno-deprecated
CCREG_SERVER_OBJECTS=ccRegSK.o  ccReg_epp.o  ccReg_server.o   pqsql.o timestamp.o
WHOIS_SERVER_OBJECTS=ccRegSK.o  ccReg_whois.o  whois_server.o 
EPP_CLIENT_OBJECTS=ccRegSK.o epp_client.o
WHOIS_CLIENT_OBJECTS=ccRegSK.o whois_client.o

all: ccWhois_server ccReg_server epp_client whois_client
.SUFFIXES:  .o

ccReg_server: $(CCREG_SERVER_OBJECTS)
	$(CXX) -o ccReg_server $(CCREG_SERVER_OBJECTS) $(LDFLAGS) $(LIBS) -lpq


ccWhois_server: $(WHOIS_SERVER_OBJECTS)
	$(CXX) -o ccWhois_server $(WHOIS_SERVER_OBJECTS) $(LDFLAGS) $(LIBS) 


epp_client: $(EPP_CLIENT_OBJECTS)
	$(CXX) -o epp_client $(EPP_CLIENT_OBJECTS) $(LDFLAGS) $(LIBS)

whois_client: $(WHOIS_CLIENT_OBJECTS)
	$(CXX) -o whois_client $(WHOIS_CLIENT_OBJECTS) $(LDFLAGS) $(LIBS)


ccRegSK.cc ccRegSK.h:
	omniidl -bcxx $(IDLFILE)

%.o: %.cc 
	$(CXX) $(CPPFLAGS) -c $<



clean:
	rm -rf *.o *_server *_client ccRegSK.cc ccRegSK.h ccReg.hh


