OBJS = register.o zone.o blacklist.o domain.o registrar.o object_impl.o \
       contact.o nsset.o auth_info.o invoice.o documents.o bank.o
EXT = ../pqsql.o ../dbsql.o ../util.o ../mailer_manager.o ../nameservice.o \
      ../ccRegSK.o ../ccRegDynSK.o
LIB = libccreg.a
LIBBOOST = -lboost_date_time -lboost_regex -lboost_program_options
LIBOMNI = -lomniORB4 -lomniDynamic4
PGINCLUDE = $(shell pg_config --includedir)
CXXFLAGS = -g -Wall -I.. -I$(PGINCLUDE)
LIBALL = $(LIB) $(LIBBOOST) $(LIBOMNI) -lpq $(EXT) 

all: $(LIB) client

$(LIB) : $(OBJS)
	$(AR) -rs $@ $(OBJS)

$(EXT):
	make -C .. $(patsubst ../%,%,$(EXT))
	
testreg: test.o $(LIB)
	$(CXX) -o $@ test.o $(LIBALL)

client: client.o $(LIB) $(EXT)
	$(CXX) -o $@ client.o $(LIBALL)

clean:
	rm -rf *.o
	rm -rf $(LIB)
