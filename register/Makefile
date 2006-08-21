OBJS = register.o zone.o blacklist.o domain.o
LIB = libccreg.a
CXXFLAGS = -g -I.. -I/usr/include/postgresql/

$(LIB) : $(OBJS)
	$(AR) -rs $@ $(OBJS)

clean:
	rm *.o
	rm $(LIB)
