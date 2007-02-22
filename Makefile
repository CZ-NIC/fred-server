
CXX = g++

CXXFLAGS = -Wall  -DSYSLOG    -DSVERSION=\"${SVN_REVISION}\"


OBJECTS = 
IDLDIR = ../../idl/trunk/
IDLFILE = $(IDLDIR)/ccReg.idl
LDFLAGS =  -L/usr/local/pgsql/lib/
LIBS =  -lomniORB4 -lomniDynamic4 -lomnithread -lpthread 
SVLIBS = $(LIBS) register/libccreg.a  -lboost_date_time -lboost_regex -lpq

CPPFLAGS =  -I/usr/local/pgsql/include/   -I/usr/include/postgresql/ \
            -I. -Wno-deprecated
COMMON_OBJECTS =  conf.o dbsql.o pqsql.o util.o log.o nameservice.o 
IDL_OBJECT =  ccRegSK.o ccRegDynSK.o
ADMIN_SERVER_OBJECTS = ccReg_adifd.o $(COMMON_OBJECTS) $(IDL_OBJECT) admin.o   tech_check.o mailer_manager.o
RIFD_SERVER_OBJECTS = ccReg_rifd.o $(COMMON_OBJECTS) $(IDL_OBJECT) ccReg_epp.o   countrycode.o messages.o tech_check.o mailer_manager.o 
ALL_SERVER_OBJECTS = ccReg_server.o $(COMMON_OBJECTS) $(IDL_OBJECT) \
 ccReg_epp.o   countrycode.o messages.o tech_check.o mailer_manager.o  admin.o   whois.o
BANKING_OBJECT = gpc.o banking.o log.o conf.o dbsql.o pqsql.o util.o csv.o
PIF_SERVER_OBJECTS = ccReg_pifd.o $(COMMON_OBJECTS)   $(IDL_OBJECT)  whois.o admin.o mailer_manager.o
EPP_CLIENT_OBJECTS= $(IDL_OBJECT)   epp_client.o nameservice.o
WHOIS_CLIENT_OBJECTS=ccRegSK.o whois_client.o nameservice.o

all:  fred_rifd fred_adifd fred_pifd banking

.SUFFIXES:  .o

ccReg_server: $(ALL_SERVER_OBJECTS)
	$(MAKE) -C register
	$(CXX) -o ccReg_server $(ALL_SERVER_OBJECTS) $(LDFLAGS)  $(SVLIBS)

fred_rifd:  $(RIFD_SERVER_OBJECTS) 
	$(MAKE) -C register
	$(CXX) -o fred_rifd  $(RIFD_SERVER_OBJECTS)   $(LDFLAGS)  $(SVLIBS)

fred_adifd: $(ADMIN_SERVER_OBJECTS)
	$(MAKE) -C register
	$(CXX) -o fred_adifd  $(ADMIN_SERVER_OBJECTS)   $(LDFLAGS)  $(SVLIBS)

fred_pifd:  $(PIF_SERVER_OBJECTS) 
	$(MAKE) -C register

	$(CXX) -o fred_pifd  $(PIF_SERVER_OBJECTS)  $(LDFLAGS)  $(SVLIBS)

epp_client: $(EPP_CLIENT_OBJECTS)
	$(CXX) -o epp_client $(EPP_CLIENT_OBJECTS) $(LDFLAGS) $(LIBS)

whois_client: $(WHOIS_CLIENT_OBJECTS)
	$(CXX) -o whois_client $(WHOIS_CLIENT_OBJECTS) $(LDFLAGS) $(LIBS)


ccRegSK.cc ccRegDynSK.cc ccRegSK.h ccReg.hh:
	omniidl -bcxx -Wba -Wbinline $(IDLFILE)

ccReg_server.o:  ccReg.hh
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DRIFD -DPIFD  -DADIF -o ccReg_ccReg_server.o -c ccReg_server.cc

ccReg_rifd.o:  ccReg.hh
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DRIFD  -o ccReg_rifd.o -c  ccReg_server.cc

ccReg_pifd.o: ccReg.hh
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DPIFD  -o ccReg_pifd.o -c  ccReg_server.cc

ccReg_adifd.o:  ccReg.hh
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DADIF  -o ccReg_adifd.o -c  ccReg_server.cc

%.o: %.cc 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS)  -c -g $<

ccReg_idl.py:
	omniidl -bpython $(IDLFILE)

banking: $(BANKING_OBJECT)
	$(CXX) -o banking  $(BANKING_OBJECT) $(LDFLAGS) -lpq


test: ccReg_idl.py
	python test.py

install_banking: banking banking.sh
	install banking /usr/local/bin/
	install banking.sh /usr/local/bin/

install: fred_rifd fred_pifd fred_adifd 
	install fred_rifd /usr/local/bin/
	install fred_pifd /usr/local/bin/
	install fred_adifd /usr/local/bin/


clean:
	make -C register clean
	rm -rf *.o fred_* *_server *_client banking ccRegDynSK.cc  ccRegSK.cc ccRegSK.h ccReg.hh ccReg_i.cc *idl.py* ccReg__POA/ ccReg/ libccreg.a 


