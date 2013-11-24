LINK_TARGET = WebSocketServer

OBJS = \
 base64.o \
 sha1.o \
 hybi10.o \
 wsHandshake.o \
 wsLuaHook.o \
 wsServer.o \
 main.o

REBUILDABLES = $(OBJS) $(LINK_TARGET)

clean :
	rm -f $(REBUILDABLES)
	echo Cleaned

all : $(LINK_TARGET)
	echo Everythings ready

$(LINK_TARGET) : $(OBJS)
	g++ -g -o $@ $^ -llua5.1

%.o : %.cpp
	g++ -c -I/usr/include/lua5.1/ $<

main.o : wsHookInterface.h wsLuaHook.h wsServer.h
wsServer.o : wsServer.h wsHandshake.h wsServerInterface.h wsHookInterface.h
wsLuaHook.o : wsLuaHook.h wsHookInterface.h wsServerInterface.h
wsHandshake.o : wsHandshake.h sha1.h base64.h
hybi10.o : hybi10.h
base64.o : base64.h
sha1.o : sha1.h
