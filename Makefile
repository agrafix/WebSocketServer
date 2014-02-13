LINK_TARGET = WebSocketServer
OBJDIR := build
CC := g++

OBJS = \
 $(OBJDIR)/base64.o \
 $(OBJDIR)/sha1.o \
 $(OBJDIR)/hybi10.o \
 $(OBJDIR)/wsHandshake.o \
 $(OBJDIR)/wsLuaHook.o \
 $(OBJDIR)/wsServer.o \
 $(OBJDIR)/main.o

REBUILDABLES = $(OBJS) $(LINK_TARGET)

$(OBJDIR):
	mkdir -p $@

clean :
	rm -f $(REBUILDABLES)
	@echo Cleaned

all : $(LINK_TARGET)
	@echo Everythings ready

$(LINK_TARGET) : $(OBJS)
	$(CC) -g -llua -o $@ $^

$(OBJDIR)/%.o : %.cpp
	$(CC) -c -I/usr/include/lua5.2/ -o $@ $^

$(OBJS) : | $(OBJDIR)
