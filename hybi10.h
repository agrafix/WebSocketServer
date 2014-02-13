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
#include <string>
#include <sstream>
#include <vector>

#include <stdlib.h>
#include <math.h>

#ifndef HYBI10_H
#define	HYBI10_H

namespace hybi10 {
    typedef unsigned char byte;
    
    struct hybiPayloadHeader {
        unsigned short opcode : 4;
        unsigned short rsv1 : 1;
        unsigned short rsv2 : 1;
        unsigned short rsv3 : 1;
        unsigned short fin : 1;
        unsigned short payload_len: 7;
        unsigned short mask : 1;
    };

    struct request {
        int exitcode;
        std::string type;
        std::string payload;
    };
    
    std::string encode(request hRequest, bool masked);
    
    request decode(std::string data);
}

#endif	/* HYBI10_H */

