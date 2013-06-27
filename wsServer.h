/**
 * ,.   ,   ,.   .   .---.         .       .  .---.                      
 * `|  /|  / ,-. |-. \___  ,-. ,-. | , ,-. |- \___  ,-. ,-. .  , ,-. ,-. 
 *  | / | /  |-' | |     \ | | |   |<  |-' |      \ |-' |   | /  |-' |   
 *  `'  `'   `-' ^-' `---' `-' `-' ' ` `-' `' `---' `-' '   `'   `-' ' 
 * 
 * Copyright 2012 by Alexander Thiemann <mail@agrafix.net>
 *
 * This file is part of WebSocketServer.
 *
 *  WebSocketServer is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  WebSocketServer is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with WebSocketServer.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef WSSERVER_H
#define	WSSERVER_H

#include <stdlib.h>

#include <string>
#include <sstream>
#include <iostream>

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
typedef int SOCKET;
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#define NO_ERROR 0
#endif

#include "./hybi10.h"
#include "./wsHandshake.h"
#include "./wsServerInterface.h"
#include "./wsHookInterface.h"

#define WS_MAX_CLIENTS 50

using namespace std;

class wsServer : public wsServerInterface {
public:
    wsServer(wsHookInterface *hook);
    virtual ~wsServer();
    
    void run();
    virtual void sendTo(int i, string message);
    virtual void sendBroadcast(string message);
    
private:
    wsHookInterface *_hook;
    
    SOCKET _sListen;
    SOCKET _sClients[WS_MAX_CLIENTS];
    
    fd_set _fdSet;
    
    // init functions
    void initClients();
    bool winSock(int action);
    bool openSocket();
    bool setNonBlocking();
    bool bindTo(int port);
    bool startListen();
    
    // mainloop functions
    void acceptClients();
    void handleClients();
    void clientRecv(int i);
    void handleRequest(int i, string request);
    
    // client functions
    void disconnectClient(int i);
    void disconnectClient(int, string reason);
    void sendRaw(int i, string message);
    void sendHybi10(int i, string type, string message, bool masked);
    
    // log & error functions
    void error(string message);
    void log(string message);
    void log(string message, int clientId);
};

#endif	/* WSSERVER_H */

