
CXX = g++

CXXFLAGS =   -DSYSLOG   -DCONFIG_FILE=\"/etc/ccReg.conf\" -DSVERSION=\"${SVN_REVISION}\"

OBJECTS = 
IDLFILE = ../idl/ccReg.idl
LDFLAGS=  -L/usr/local/pgsql/lib/
LIBS=  -lomniORB4 -lomniDynamic4 -lomnithread -lpthread
CPPFLAGS =  -I/usr/local/pgsql/include/   -I/usr/include/postgresql/  -I.  -Wno-deprecated
CCREG_SERVER_OBJECTS = \
    ccRegSK.o ccRegDynSK.o  ccReg_epp.o  ccReg_server.o  \
    dbsql.o pqsql.o util.o status.o conf.o log.o admin.o whois.o \
    nameservice.o
EPP_CLIENT_OBJECTS=ccRegSK.o ccRegDynSK.o  epp_client.o nameservice.o
WHOIS_CLIENT_OBJECTS=ccRegSK.o whois_client.o nameservice.o


all:  ccReg_server epp_client whois_client
.SUFFIXES:  .o

ccReg_server: $(CCREG_SERVER_OBJECTS)
	$(MAKE) -C register
#	ar -rs libccreg.a $(CCREG_SERVER_OBJECTS)
	$(CXX) -o ccReg_server $(CCREG_SERVER_OBJECTS) $(LDFLAGS) $(LIBS) register/libccreg.a -lpq


epp_client: $(EPP_CLIENT_OBJECTS)
	$(CXX) -o epp_client $(EPP_CLIENT_OBJECTS) $(LDFLAGS) $(LIBS)

whois_client: $(WHOIS_CLIENT_OBJECTS)
	$(CXX) -o whois_client $(WHOIS_CLIENT_OBJECTS) $(LDFLAGS) $(LIBS)


ccRegSK.cc ccRegDynSK.cc ccRegSK.h:
	omniidl -bcxx -Wba   -Wbinline $(IDLFILE)

%.o: %.cc 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS)  -c -g $<

ccReg_idl.py:
	omniidl -bpython $(IDLFILE)

test: ccReg_idl.py
	python test.py

install:
	install ccReg_server /usr/local/bin/ccReg

clean:
	rm -rf *.o *_server *_client ccRegDynSK.cc  ccRegSK.cc ccRegSK.h ccReg.hh ccReg_i.cc *idl.py* ccReg__POA/ ccReg/ libccreg.a


