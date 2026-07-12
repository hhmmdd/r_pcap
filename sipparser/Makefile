.SUFFIXES: .o .c

CC	= gcc
DEFLIST = -DUNIX -DLINUX -DF_MALLOC -DPKG_MALLOC -DMULTITHREAD -D_REENTRANT
INCLUDE = -I/usr/local/include
LIB     = -L/usr/local/lib
LIBS	= -lpthread -ldl -lnsl
LINKS	= 


PROJECT	= ../../../_bin/libsipparser.a
SOURCES	= $(wildcard *.c parser/*.c parser/contact/*.c mem/*.c sdp/*.c)
OBJECTS = $(SOURCES:%.c=%.o)

$(PROJECT): $(OBJECTS)
	ar -r $@ $^

%.o: %.c
	$(CC) -c $(CCFLAGS) $(INCLUDE) $(DEFLIST) -o $@ $<

clean:
	rm -f $(PROJECT) *.o parser/*.o parser/contact/*.o mem/*.o sdp/*.o


