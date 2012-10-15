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

#ifndef WSLUAHOOK_H
#define	WSLUAHOOK_H

#include <stdio.h>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <fstream>

extern "C" {
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "./wsHookInterface.h"
#include "./wsServerInterface.h"

using namespace std;

class wsLuaHook : public wsHookInterface {
public:
    virtual ~wsLuaHook();
    
    virtual void hServerStartup(wsServerInterface *server);
    virtual void hServerShutdown();
    
    virtual void hClientConnect(int i);
    virtual void hClientDisconnect(int i);
    virtual void hClientMessage(int i, std::string message);
    
private:
    lua_State* _L;
    static wsServerInterface* _server;
    
    // functions to be called from lua
    static int lSend(lua_State *L);
    static int lSendAll(lua_State *L);
};

#endif	/* WSLUAHOOK_H */

