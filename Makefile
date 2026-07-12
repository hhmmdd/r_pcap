CC	= g++
DEFLIST = -DPASN_NOPRINTON -DPKG_MALLOC -DUNIX -DLINUX -DMULTITHREAD -D_REENTRANT
INCLUDE = -I/usr/local/include
LIB     = -L/usr/local/lib -L/usr/lib64 -L../../_bin
LIBS	= -lpcap -lpt -lpthread -ldl -lnsl
LINKS	= -shared
DEBUG   = -g

PROJECT	= ../../_bin/r_pcap.so
SOURCES	= $(wildcard *.cpp ../h323lib/*.cpp ../h323lib/*.cxx ../h323lib/asn/*.cxx sipparser/*.c sipparser/parser/*.c sipparser/parser/contact/*.c sipparser/parser/digest/*.c sipparser/mem/*.c sipparser/sdp/*.c)
SOURCE1 = $(filter-out ../h323lib/asn/h235_t.cxx, $(SOURCES))
SOURCE2 = $(SOURCE1:%.cpp=%.o)
SOURCE3 = $(SOURCE2:%.cxx=%.o)
OBJECTS = $(SOURCE3:%.c=%.o)

$(PROJECT): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LINKS) $(LIB) $(LIBS) -o $@

%.o: %.cpp
	$(CC) -c -fPIC -std=c++11 -Wno-write-strings $(CCFLAGS) $(DEBUG) $(INCLUDE) $(DEFLIST) -o $@ $<

%.o: %.cxx
	$(CC) -c -fPIC -std=c++11 -Wno-write-strings $(CCFLAGS) $(DEBUG) $(INCLUDE) $(DEFLIST) -o $@ $<

%.o: %.c
	gcc -c -fPIC -Wno-pointer-to-int-cast $(CCFLAGS) $(DEBUG) $(INCLUDE) $(DEFLIST) -o $@ $<

clean:
	rm -f $(PROJECT) *.o ../h323lib/*.o ../h323lib/asn/*.o sipparser/*.o sipparser/parser/*.o sipparser/parser/contact/*.o sipparser/parser/digest/*.o sipparser/mem/*.o sipparser/sdp/*.o


