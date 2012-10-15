/**
 * ,.   ,   ,.   .   .---.         .       .  .---.                      
 * `|  /|  / ,-. |-. \___  ,-. ,-. | , ,-. |- \___  ,-. ,-. .  , ,-. ,-. 
 *  | / | /  |-' | |     \ | | |   |<  |-' |      \ |-' |   | /  |-' |   
 *  `'  `'   `-' ^-' `---' `-' `-' ' ` `-' `' `---' `-' '   `'   `-' ' 
 *   
 * A WebSocketServer written in C++
 * Designed and tested for draft-ietf-hybi-thewebsocketprotocol-17
 * @see https://tools.ietf.org/html/draft-ietf-hybi-thewebsocketprotocol-17
 * 
 * Version 0.2beta
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

#include "./wsHookInterface.h"
#include "./wsLuaHook.h"
#include "./wsServer.h"

int main(int argc, char** argv)
{
    wsLuaHook hook;
    
    wsServer server(&hook);
    
    server.run();
}