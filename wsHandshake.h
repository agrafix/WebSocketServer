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

#ifndef WSHANDSHAKE_H
#define	WSHANDSHAKE_H

#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "./sha1.h"
#include "./base64.h"

using namespace std;

class wsHandshake {
public:
    wsHandshake(string payload);
    virtual ~wsHandshake();
    
    void generateResponse();
    bool isSuccess();
    
    string getResponse();
    
private:
    string _payload;
    
    string _get;
    int _version;
    string _key;
    
    bool _success;
    string _responsePayload;
    
    string extractValue(string line, string key);
};

#endif	/* WSHANDSHAKE_H */

