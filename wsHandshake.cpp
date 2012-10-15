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

#include "wsHandshake.h"

/**
 * init the handshake
 * @param payload
 */
wsHandshake::wsHandshake(string payload) { 
    _payload = payload;
    _success = false;
}

/**
 * destructor. does nothing
 */
wsHandshake::~wsHandshake() { }

/**
 * extract a value in the key: value format
 * @param line
 * @param key
 * @return 
 */
string wsHandshake::extractValue(string line, string key) {
    key = key.append(": ");
    
    size_t i = key.length();
    
    return line.substr(i);
}

/**
 * return handshake response, ready to send to client
 * @return 
 */
string wsHandshake::getResponse() {
    return _responsePayload;
}

/**
 * does getResponse contain a valid response?
 * @return 
 */
bool wsHandshake::isSuccess() {
    return _success;
}

/**
 * build the handshake response
 */
void wsHandshake::generateResponse() {
    // line by line
    size_t lastPos = 0;
    string line;
    string delim = "\r\n";
    
    while (lastPos != string::npos) {
        size_t start = lastPos;
        lastPos = _payload.find(delim, lastPos);
        if (lastPos == string::npos) {
            break;
        }
        
        line = _payload.substr(start, lastPos-start);
        remove(line.begin(), line.end(), '\r');
        remove(line.begin(), line.end(), '\t');
        remove(line.begin(), line.end(), '\n');
        
        // GET Line
        if (line[0] == 'G' && line[1] == 'E' && line[2] == 'T') {
            // get part
            size_t end = _payload.find(" ", 4);
            
            _get = line.substr(4, end-4);
            //cout << "GET |" << _get << "|\n";
        }
        
        // version
        if (line.find("Sec-WebSocket-Version") != string::npos) {
            string val = extractValue(line, "Sec-WebSocket-Version");
            
            _version = atoi(val.c_str());
            cout << "VERS |" << _version << "| \n";
        }
        
        // key
        if (line.find("Sec-WebSocket-Key") != string::npos) {
            _key = extractValue(line, "Sec-WebSocket-Key");
            
            cout << "KEY |" << _key << "|\n";
        }
        
        // other params are unhandled
        /* @TODO
         Upgrade:
            Connection:
            Host:
            Sec-WebSocket-Origin:
         */
        lastPos += 2;
    }
    
    // now check params
    if (_version < 6) {
        _success = false;
        return;
    }
    
    // now calculate key
    unsigned char hash[20];
    char hex[41];
    
    _key.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    sha1::calc(_key.c_str(), _key.length(), hash);
    string responseKey = base64_encode(hash, 20);
    
    // build payload
    _success = true;
    
    _responsePayload = "HTTP/1.1 101 Switching Protocols\r\n";
    _responsePayload.append("Upgrade: websocket\r\n");
    _responsePayload.append("Connection: Upgrade\r\n");
    _responsePayload.append("Sec-WebSocket-Accept: " + responseKey + "\r\n\r\n");
    
    return;
}
