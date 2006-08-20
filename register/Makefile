OBJS = register.o zone.o blacklist.o domain.o
LIB = libccreg.a
CXXFLAGS = -g -I..

$(LIB) : $(OBJS)
	$(AR) -rs $@ $(OBJS)

clean:
	rm *.o
	rm $(LIB)
