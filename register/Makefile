OBJS = register.o zone.o blacklist.o domain.o registrar.o
EXT = ../pqsql.o ../dbsql.o ../util.o -lpq
LIB = libccreg.a
CXXFLAGS = -g -Wall -I.. -I/usr/include/postgresql/

all: $(LIB) testreg

$(LIB) : $(OBJS)
	$(AR) -rs $@ $(OBJS)

testreg: test.o $(OBJS)
	$(CXX) -o $@ test.o $(OBJS) $(EXT)

clean:
	rm *.o
	rm $(LIB)
