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

#include <stdio.h>

#include "wsServer.h"

/**
 * init the WebSocketServer
 */
wsServer::wsServer(wsHookInterface *hook) { 
    log("===============================");
    log("WebSocketServer  Copyright (C) 2012 Alexander Thiemann <mail@agrafix.net>");
    log("This program comes with ABSOLUTELY NO WARRANTY");
    log("This is free software, and you are welcome to redistribute it");
    log("under certain conditions; check the provided license file.");
    log("===============================");
    log("visit www.websocketserver.de");
    
    // store hook
    _hook = hook;
    
    // init clients
    initClients();
    
    // wsaStartup
    if (!winSock(1)) {
        error("WSAStartup failed.");
    }
    
    // open socket
    if (!openSocket()) {
        error("Error opening the socket.");
    }
    
    // set it to nonblocking
    if (!setNonBlocking()) {
        error("Failed to set socket to nonblocking-io mode");
    }
    
    // bind to port 8085
    if (!bindTo(8085)) {
        error("Failed to bind to port 8085");
    }
    
    // listen
    if (!startListen()) {
        error("Failed to listen on port 8085");
    }
    
    log("Server started. Now listening on port 8085");
   
    // call server startup hook
    _hook->hServerStartup(this);
}

/**
 * stop server
 */
wsServer::~wsServer() { 
    // close socket
    #ifdef WIN32
    closesocket(_sListen);
    #else
    close(_sListen);
    #endif
    
    // cleanup
    winSock(2);
    
    // server closed
    _hook->hServerShutdown();
}

/**
 * main server loop, is blocking and will block until the server crashes...
 */
void wsServer::run() {
    while (true) {
        FD_ZERO(&_fdSet);
        FD_SET(_sListen, &_fdSet);
        
        // add valid sockets to fdSet
        for (int i = 0; i < WS_MAX_CLIENTS; i++) {
            if (_sClients[i] != INVALID_SOCKET) {
                FD_SET(_sClients[i], &_fdSet);
            }
        }
        
        // select-call
        if (select(0,&_fdSet,NULL,NULL,NULL) == SOCKET_ERROR) {
            error("Error using select()");
        }
        
        if (FD_ISSET(_sListen, &_fdSet)) {
            // accept new clients
            acceptClients();
        }
        
        // handle clients
        handleClients();
    }
}

/**
 * accept new clients
 */
void wsServer::acceptClients() {
    for (int i = 0; i < WS_MAX_CLIENTS; i++) {
        if (_sClients[i] == INVALID_SOCKET) {
            _sClients[i] = accept(_sListen, NULL, NULL);
            
            /* Detect dead connections */
            int   keepalive = 1 ;
            setsockopt(_sClients[i], SOL_SOCKET, SO_KEEPALIVE,
                  (char *)&keepalive,
                  sizeof(keepalive)) ;
            
            log("new connection", i);
            break;
        }
    }
}

/**
 * handle clients
 */
void wsServer::handleClients() {
    for (int i = 0; i < WS_MAX_CLIENTS; i++) {
        // skip empty slots and clients that didn't send anything
        if (_sClients[i] == INVALID_SOCKET) {
            continue;
        }
        
        if (FD_ISSET(_sClients[i], &_fdSet)) {
            FD_CLR(_sClients[i], &_fdSet);
            clientRecv(i);
        }
    }
}

/**
 * handle signle client
 * @param i
 */
void wsServer::clientRecv(int i) {
    stringstream requestStream;
    char buff[256];
    
    long status = recv(_sClients[i], buff, 256, 0);
    while (status > 0) {
        buff[status] = '\0'; // terminate input
        requestStream.write(buff, status);
        
        // load next chunk
        status = recv(_sClients[i], buff, 256, 0);
    }
    
    if (status == 0) {
        cout << "Errno: " << errno << "\n";
        cout << "Client " << i << " quit. Status: 0\n";
        disconnectClient(i, "Client Status is 0");
        return;
    }
    
    if (requestStream.str().length() > 0) {
        cout << "Client " << i << ": Status: " << status << "\n";
        cout << "Data: " << requestStream.str() << "\n";
    
        handleRequest(i, requestStream.str());
    }
}

/**
 * handle a client request
 * @param i client-ID
 * @param request request-body
 */
void wsServer::handleRequest(int i, string request) {
    if (request[0] == 'G' &&
        request[1] == 'E' &&
        request[2] == 'T') {
        
        // handshake!
        log("handshake", i);
        
        wsHandshake handshake(request);
        handshake.generateResponse();
        
        if (!handshake.isSuccess()) {
            log("handshake failed", i);
            disconnectClient(i, "Handshake failed");
            return;
        }
        
        sendRaw(i, handshake.getResponse());
        log("handshake complete", i);
        
        // hook new client
        _hook->hClientConnect(i);
        
        return;
    }
    
    // no handshake, so ready hybi10 format
    hybi10::request r = hybi10::decode(request);
    
    // handle invalid requests
    if (r.exitcode != 0) {
        log("sent invalid hybi10 data!", i);
        sendHybi10(i, "close", "Goodbye...", false);
        disconnectClient(i, "invalid hybi10 data");
        return;
    }
    
    // switch at type
    if (r.type == "ping") {
        sendHybi10(i, "pong", r.payload, false);
    }
    else if (r.type == "pong") {
        // i dont ping?!
        log("recieved a pong but never pinged?!");
    }
    else if (r.type == "close") {
        log("close connection", i);
        disconnectClient(i, "asked to close connection");
    }
    else if (r.type == "text") {
        log("Got message: " + r.payload, i);
        // call hook
        _hook->hClientMessage(i, r.payload);
    }
}

/**
 * broadcast message to all connected clients
 * @param message
 */
void wsServer::sendBroadcast(string message) {
    for (int i = 0; i < WS_MAX_CLIENTS; i++) {
        if (_sClients[i] == INVALID_SOCKET) {
            continue;
        }
        
        sendTo(i, message);
    }
}

/**
 * send hyb10-encoded message to client
 * @param i
 * @param message
 */
void wsServer::sendTo(int i, string message) {
    sendHybi10(i, "text", message, false);
}

/**
 * send data in hybi10 format
 * @param i
 * @param type
 * @param message
 * @param masked
 */
void wsServer::sendHybi10(int i, string type, string message, bool masked) {
    hybi10::request response;
    response.type = type;
    response.payload = message;
    
    sendRaw(i, hybi10::encode(response, masked));
}

/**
 * send a raw tcp packet to a client
 * @param i
 * @param message
 */
void wsServer::sendRaw(int i, string message) {
    char *response;
    response = const_cast<char*>(message.c_str());
    
    cout << "Sending: " << message << " to " << i << "\n";
    
    if (_sClients[i] == INVALID_SOCKET) {
        return;
    }
    
    send(_sClients[i], response, (int)strlen(response), 0);
}

/**
 * disconnect clients
 * @param i
 */
void wsServer::disconnectClient(int i) {
    disconnectClient(i, "-");
    /*log("disconnected", i);
    
    if (_sClients[i] != INVALID_SOCKET) {
        closesocket(_sClients[i]);
        _sClients[i] = INVALID_SOCKET;
    }
    
    _hook->hClientDisconnect(i);*/
}

/**
 * disconnect a client and indicate reason
 * @param i
 * @param reason
 */
void wsServer::disconnectClient(int i, string reason) {
    log("disconnected. Reason: " + reason, i);
    
    if (_sClients[i] != INVALID_SOCKET) {
#ifdef WIN32
        closesocket(_sClients[i]);
#else
        close(_sClients[i]);
#endif
        _sClients[i] = INVALID_SOCKET;
    }
    
    _hook->hClientDisconnect(i);
}

/**
 * start to listen for data
 * @return false on failure, true on success
 */
bool wsServer::startListen() {
    if(listen(_sListen,10) != 0)
    {
        return false;
    }
    
    return true;
}

/**
 * bind socket to a port
 * @param port
 * @return false on failure, true on success
 */
bool wsServer::bindTo(int port) {
    
    sockaddr_in local;
    
    // set addr
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons((u_short)port);
    
    if(bind(_sListen,(sockaddr*)&local,sizeof(local))!=0)
    {
        return false;
    }
    
    return true;
}

/**
 * open socket
 * @return false on failure, true on success
 */
bool wsServer::openSocket() {
    _sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    
    if (_sListen == SOCKET_ERROR) {
        return false;
    }
    
    return true;
}

/**
 * set the socket to nonblocking-IO
 * @return false on failure, true on success
 */
bool wsServer::setNonBlocking() {
    u_long iMode = 1;
#ifdef WIN32
    int iResult = ioctlsocket(_sListen, FIONBIO, &iMode);
    if (iResult != NO_ERROR) {
        return false;
    }
#else
    ioctl(_sListen, FIONBIO, &iMode);
#endif
    
    return true;
}

/**
 * set all client sockets to INVALID_SOCKET
 */
void wsServer::initClients() {
    for (int i = 0; i < WS_MAX_CLIENTS; i++) {
        _sClients[i] = INVALID_SOCKET;
    }
}

/**
 * WSA function, only needed on windows systems
 * @param action 1: startup, 2: cleanup
 * @return false on error, true on success
 */
bool wsServer::winSock(int action) {
#ifdef WIN32
    if (action == 1) {
        WSADATA w;
        if(int result = WSAStartup(MAKEWORD(2,2), &w) != 0)
        {
            cout << "Winsock 2 couldn't be launched! Error #" << result << endl;
            return false;
        }
    }
    else if (action == 2) {
        WSACleanup();
    }
#endif
    return true;
}

/**
 * Handle an error
 * @param message
 */
void wsServer::error(string message) {
    cout << "[ERROR] " << message << "\n";
    exit(1);
}

/**
 * handle a message
 * @param message
 */
void wsServer::log(string message) {
    cout << "[MESSAGE] " << message << "\n";
}

/**
 * handle a message belonging to a client
 * @param message
 * @param clientId
 */
void wsServer::log(string message, int clientId) {
    cout << "[MESSAGE] [CLI: " << clientId << "] " << message << "\n";
}
