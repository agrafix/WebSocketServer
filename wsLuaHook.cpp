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

#include "wsLuaHook.h"

wsServerInterface* wsLuaHook::_server = 0;

wsLuaHook::~wsLuaHook() { }

void wsLuaHook::hServerStartup(wsServerInterface *server) {
    // server pointer
    wsLuaHook::_server = server;
    
    // check if lua-main-script exists
    ifstream ifile("main.lua");
    if (!ifile) {
        cout << "FATAL Error: main.lua not found.\n";
        return;
    }
    ifile.close();
    
    // load main lua
    _L = luaL_newstate();
    luaL_openlibs(_L);
    
    // register funcs
    lua_register(_L, "_send", lSend);
    lua_register(_L, "_sendAll", lSendAll);
    
    // parse main file
    luaL_dofile(_L, "main.lua");
}

void wsLuaHook::hServerShutdown() {
    // close lua
    lua_close(_L);
}

void wsLuaHook::hClientConnect(int i) {
    lua_getglobal(_L, "on_connect");
    
    if (!lua_isfunction(_L,lua_gettop(_L))) {
        cout << "FATAL ERROR: Missing on_connect() in lua-Script!";
        return;
    }
    
    lua_pushnumber(_L, i);
    
    lua_call(_L, 1, 0);
}

void wsLuaHook::hClientDisconnect(int i) {
    lua_getglobal(_L, "on_disconnect");
    
    if (!lua_isfunction(_L,lua_gettop(_L))) {
        cout << "FATAL ERROR: Missing on_disconnect() in lua-Script!";
        return;
    }
    
    lua_pushnumber(_L, i);
    
    lua_call(_L, 1, 0);
}

void wsLuaHook::hClientMessage(int i, std::string message) {
    lua_getglobal(_L, "main");
    
    if (!lua_isfunction(_L,lua_gettop(_L))) {
        cout << "FATAL ERROR: Missing main() in lua-Script!";
        return;
    }
    
    lua_pushnumber(_L, i);
    lua_pushstring(_L, message.c_str());
    
    lua_call(_L, 2, 0);
}

int wsLuaHook::lSend(lua_State *L) {
    int args = lua_gettop(L);
    
    if (args != 2) {
        return 0;
    }
    
    int clientId = lua_tonumber(L, 1);
    string message = lua_tostring(L, 2);
    
    //_srvSend(false, clientId, message);
    wsLuaHook::_server->sendTo(clientId, message);
    
    return 0;
}

int wsLuaHook::lSendAll(lua_State *L) {
    int args = lua_gettop(L);
    
    if (args != 1) {
        return 0;
    }
    
    string message = lua_tostring(L, 1);
    
    wsLuaHook::_server->sendBroadcast(message);
    
    return 0;
}