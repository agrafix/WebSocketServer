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

#ifndef WSSERVERINTERFACE_H
#define	WSSERVERINTERFACE_H

#include <string>

class wsServerInterface {
public:
    virtual void sendTo(int i, std::string message) = 0;
    virtual void sendBroadcast(std::string message) = 0;
};

#endif	/* WSSERVERINTERFACE_H */

