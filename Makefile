
CXX = g++

CXXFLAGS = -Wall -DSYSLOG -DCONFIG_FILE=\"/etc/ccReg.conf\" \
           -DSVERSION=\"${SVN_REVISION}\"

OBJECTS = 
IDLFILE = ../../idl/branches/devel/ccReg.idl
LDFLAGS =  -L/usr/local/pgsql/lib/
LIBS =  -lomniORB4 -lomniDynamic4 -lomnithread -lpthread 
SVLIBS = $(LIBS) register/libccreg.a  -lboost_date_time -lpq

CPPFLAGS =  -I/usr/local/pgsql/include/   -I/usr/include/postgresql/ \
            -I.  -Wno-deprecated
ADMIN_SERVER_OBJECTS = \
    ccRegSK.o admin.o log.o conf.o dbsql.o pqsql.o util.o admin_server.o \
    nameservice.o
CCREG_SERVER_OBJECTS = \
    ccRegSK.o ccRegDynSK.o  ccReg_epp.o  ccReg_server.o  \
    dbsql.o pqsql.o util.o conf.o  log.o   \
    nameservice.o countrycode.o messages.o 
PIF_SERVER_OBJECTS = \
    ccRegSK.o ccRegDynSK.o  ccReg_epp.o  pif_server.o  \
    dbsql.o pqsql.o util.o status.o conf.o  log.o  whois.o admin.o \
    nameservice.o countrycode.o messages.o 
EPP_CLIENT_OBJECTS=ccRegSK.o ccRegDynSK.o  epp_client.o nameservice.o
WHOIS_CLIENT_OBJECTS=ccRegSK.o whois_client.o nameservice.o

all:  fred_rifd fred_adifd fred_pifd

.SUFFIXES:  .o

ccReg_server: $(CCREG_SERVER_OBJECTS)
	$(CXX) -o ccReg_server $(CCREG_SERVER_OBJECTS) $(LDFLAGS)  $(LIBS)  -lpq

fred_adifd: $(ADMIN_SERVER_OBJECTS)
	$(MAKE) -C register
	$(CXX) -o fred_adifd $(ADMIN_SERVER_OBJECTS) $(LDFLAGS) $(SVLIBS)

fred_pifd: $(PIF_SERVER_OBJECTS)
	$(MAKE) -C register
	$(CXX) -o fred_pifd $(PIF_SERVER_OBJECTS) $(LDFLAGS) $(SVLIBS)

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
	make -C register clean
	rm -rf *.o *_server *_client ccRegDynSK.cc  ccRegSK.cc ccRegSK.h ccReg.hh ccReg_i.cc *idl.py* ccReg__POA/ ccReg/ libccreg.a


