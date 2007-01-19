OBJS = register.o zone.o blacklist.o domain.o registrar.o object_impl.o \
       contact.o nsset.o auth_info.o invoice.o
EXT = ../pqsql.o ../dbsql.o ../util.o -lpq
LIB = libccreg.a
CXXFLAGS = -g -Wall -I.. -I/usr/include/postgresql/ -DTIMECLOCK

all: $(LIB) testreg

$(LIB) : $(OBJS)
	$(AR) -rs $@ $(OBJS)

testreg: test.o $(OBJS)
	$(CXX) -o $@ test.o $(OBJS) $(EXT) -lboost_date_time -lboost_regex

clean:
	rm -rf *.o
	rm -rf $(LIB)
