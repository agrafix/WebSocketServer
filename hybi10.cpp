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

#include "string.h"
#include "hybi10.h"

namespace hybi10 {
    
    namespace { // local namespace
        unsigned char randomChar() {
            static const char alphanum[] =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";

            return alphanum[rand() % (sizeof(alphanum) - 1)];
        }

        
    } // end
    
    std::string encode(request hRequest, bool masked) {
        std::stringstream sstr;
        
        // set header flags: fin, rsv, opcode
        unsigned char head = 0;
        
        if (hRequest.type == "text") {
            head = 129; // 10000001
        }
        else if (hRequest.type == "close") {
            head = 136; // 10001000
        }
        else if (hRequest.type == "ping") {
            head = 137; // 10001001
        }
        else if (hRequest.type == "pong") {
            head = 138; // 10001010
        }
        
        sstr << head;
        
        // set payload info
        size_t length = hRequest.payload.length();
        
        if (length > 65535) {
            // 9 bytes needed
            unsigned long len = length;
            
            unsigned char b1 = (masked ? 255 : 127);
            sstr << b1;
            
            for (int i = 7; i >= 0; i--) {
                unsigned char b = 0;
                
                for (int j = 0; j < 8; j++) {
                    unsigned char shift = 0x01 << j;
                    shift = shift << (8 * i);
                    
                    b += pow(2, j) * (len&shift);
                }
                
                if (i == 7 && b > 127) // frame too big
                {
                    return "";
                }
                
                sstr << b;
            }
            
        }
        else if (length > 125) {
            // 3 bytes needed
            unsigned short len = length;
            
            unsigned char b1 = (masked ? 254 : 126);
            
            unsigned char b2 = 128 * (len&0x8000) + 64 * (len&0x4000) + 32 * (len&0x2000) + 16 * (len&0x1000);
            b2 += 8 * (len&0x800) + 4*(len&0x400) + 2*(len&0x200) + (len&0x100);
            
            unsigned char b3 = 128 * (len&0x80) + 64 * (len&0x40) + 32 * (len&0x20) + 16 * (len&0x10);
            b3 += 8 * (len&0x08) + 4*(len&0x04) + 2*(len&0x02) + (len&0x01);
            
            sstr << b1;
            sstr << b2;
            sstr << b3;
        }
        else {
            // 1 byte needed
            unsigned char b = (masked ? length + 128 : length);  
            sstr << b;
        }
        
        unsigned char mask[4];
        
        if (masked) {
            // generate mask
            for (int i = 0; i < 4; i++) {
                mask[i] = randomChar();
                sstr << mask[i];
            }
        }
        
        for (int i = 0; i < length; i++) {
            unsigned char chr = hRequest.payload[i];
            unsigned char res = (masked ? (chr ^ mask[i % 4]) : chr);
            
            sstr << res;
        }
        
        return sstr.str();
    }
    
    request decode(std::string data) {
        // output
        request Decoded;
        Decoded.type = "Unknown";
        Decoded.exitcode = 0;
        Decoded.payload = "";
        
        // header format
        hybiPayloadHeader* pload;
        pload = (struct hybiPayloadHeader*)data.substr(0, 2).c_str();
        
        printf("Header: OPCODE %x MASKED? %d, LENGTH: %d\n", pload->opcode, pload->mask, pload->payload_len);
        
        // check the op-code
        if (pload->opcode == 1) {
            Decoded.type = "text";
        } else if (pload->opcode == 8) {
            Decoded.type = "close";
        } else if (pload->opcode == 9) {
            Decoded.type = "ping";
        } else if (pload->opcode == 10) {
            Decoded.type = "pong";
        } else {
            Decoded.type = "unknown opcode";
            Decoded.exitcode = 1003;
            return Decoded;
        }
        
        std::string payloadBody = data.substr(2); // remove 2 byte long header
        std::string payloadMask;
        
        int payload_offset = 0;

        if (pload->payload_len == 126)
        {
            // 2 byte payload length
            payload_offset += 2;
        }
        else if (pload->payload_len == 127)
        {
            // 8 byte payload length
            payload_offset += 8;
        }
        
        printf("Payload offset: %d \n", payload_offset);
        
        if ((data[1]&0x80))
        {
            // extract the mask
            payloadMask = payloadBody.substr(payload_offset, 4);
            payload_offset += 4; // mask key is 4 bytes long
        }
        
        printf("Payload offset: %d \n", payload_offset);
        
        payloadBody = payloadBody.substr(payload_offset);
        printf("MASK: %s PAYLOAD: %s \n", payloadMask.c_str(), payloadBody.c_str());
        
        // unmask payloadBody
        int payloadBodyLength = payloadBody.length();
        
        std::stringstream unm;
        for (int i = 0; i < payloadBodyLength; i++) {
            unsigned char d = payloadBody[i];
            unsigned char m = payloadMask[i % 4];
            unsigned char unmasked = d ^ m;
            
            unm << unmasked;
        }
        
        printf("UNMASKED: %s \n", unm.str().c_str());
        
        Decoded.payload = unm.str();
        return Decoded;
    }
}
