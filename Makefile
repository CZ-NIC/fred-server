
CXX = g++

CXXFLAGS = -Wall -DSYSLOG -DCONFIG_FILE=\"/etc/ccReg.conf\" \
           -DSVERSION=\"${SVN_REVISION}\"

OBJECTS = 
IDLDIR = ../../idl/branches/devel/
IDLFILE = $(IDLDIR)/ccReg.idl
LDFLAGS =  -L/usr/local/pgsql/lib/
LIBS =  -lomniORB4 -lomniDynamic4 -lomnithread -lpthread 
SVLIBS = $(LIBS) register/libccreg.a  -lboost_date_time -lpq

CPPFLAGS =  -I/usr/local/pgsql/include/   -I/usr/include/postgresql/ \
            -I. -Wno-deprecated
COMMON_OBJECTS = conf.o dbsql.o pqsql.o util.o log.o nameservice.o \
    ccRegSK.cc ccRegDynSK.o mailer_manager.o
ADMIN_SERVER_OBJECTS = $(COMMON_OBJECTS) admin.o  admin_server.o
CCREG_SERVER_OBJECTS = $(COMMON_OBJECTS) ccReg_epp.o  ccReg_server.o  \
   countrycode.o messages.o 
BANKING_OBJECT = gpc.o banking.o log.o conf.o dbsql.o pqsql.o util.o 
PIF_SERVER_OBJECTS = $(COMMON_OBJECTS) ccReg_epp.o  pif_server.o  \
   whois.o admin.o countrycode.o messages.o 
EPP_CLIENT_OBJECTS=ccRegSK.o ccRegDynSK.o  epp_client.o nameservice.o
WHOIS_CLIENT_OBJECTS=ccRegSK.o whois_client.o nameservice.o

all:  fred_rifd fred_adifd fred_pifd

.SUFFIXES:  .o

fred_rifd: $(CCREG_SERVER_OBJECTS) ccReg.hh
	$(MAKE) -C register
	$(CXX) -o fred_rifd $(CCREG_SERVER_OBJECTS) $(LDFLAGS)  $(SVLIBS)

fred_adifd: $(ADMIN_SERVER_OBJECTS) ccReg.hh
	$(MAKE) -C register
	$(CXX) -o fred_adifd $(ADMIN_SERVER_OBJECTS) $(LDFLAGS) $(SVLIBS)

fred_pifd: $(PIF_SERVER_OBJECTS) ccReg.hh
	$(MAKE) -C register
	$(CXX) -o fred_pifd $(PIF_SERVER_OBJECTS) $(LDFLAGS) $(SVLIBS)

epp_client: $(EPP_CLIENT_OBJECTS)
	$(CXX) -o epp_client $(EPP_CLIENT_OBJECTS) $(LDFLAGS) $(LIBS)

whois_client: $(WHOIS_CLIENT_OBJECTS)
	$(CXX) -o whois_client $(WHOIS_CLIENT_OBJECTS) $(LDFLAGS) $(LIBS)


ccRegSK.cc ccRegDynSK.cc ccRegSK.h ccReg.hh:
	omniidl -bcxx -Wba -Wbinline $(IDLFILE)

%.o: %.cc 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS)  -c -g $<

ccReg_idl.py:
	omniidl -bpython $(IDLFILE)

banking: $(BANKING_OBJECT)
	$(CXX) -o banking_gpc  $(BANKING_OBJECT) $(LDFLAGS) -lpq


test: ccReg_idl.py
	python test.py

install: fred_rifd
	install fred_rifd /usr/local/bin/ccReg

clean:
	make -C register clean
	rm -rf *.o fred_* *_server *_client ccRegDynSK.cc  ccRegSK.cc ccRegSK.h ccReg.hh ccReg_i.cc *idl.py* ccReg__POA/ ccReg/ libccreg.a 


