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
    
    std::string encode2(request hRequest, bool masked) {
        std::vector<byte> packet;
        
        // set header flags: fin, rsv, opcode
        byte head;
        
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
        
        // add to packet
        packet.push_back(head);
        
        // determine packet length
        unsigned int payloadLength = hRequest.payload.length();
        
        if (payloadLength < 126) {
            packet.push_back((byte) (masked ? payloadLength + 128 : payloadLength));
        }
        else if (payloadLength >= 126 && payloadLength < 0xFFFF) {
            packet.push_back((byte) (masked ? 254 : 126));
            packet.push_back((byte) ((payloadLength >> 8) & 0xFF));
            packet.push_back((byte) (payloadLength & 0xFF));
        }
        else if (payloadLength >= 0xFFFF) {
            packet.push_back((byte) (masked ? 255 : 127));
            
            unsigned long lLen = (unsigned long) payloadLength;
            
            for (int i = 7; i >= 0; i--) {
                unsigned char b;
                
                for (int j = 0; j < 8; j++) {
                    unsigned char shift = 0x01 << j;
                    shift = shift << (8 * i);
                    
                    b += pow(2, j) * (lLen & shift);
                }
                
                if (i == 7 && b > 127) // frame too big
                {
                    return "";
                }
                
                packet.push_back(b);
            }
        }
        
        unsigned char mask[4];
        
        if (masked) {
            // generate mask
            for (int i = 0; i < 4; i++) {
                mask[i] = randomChar();
                packet.push_back(mask[i]);
            }
        }
        
        for (int i = 0; i < payloadLength; i++) {
            byte chr = hRequest.payload[i];
            packet.push_back((masked ? (chr ^ mask[i % 4]) : chr));
        }
        
        std::string pkt( packet.begin(), packet.end() );
        return pkt;
    }
    
    std::string encode(request hRequest, bool masked) {
        std::stringstream sstr;
        
        // set header flags: fin, rsv, opcode
        unsigned char head;
        
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
                unsigned char b;
                
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
        
        /*
        request Decoded;
        Decoded.type = "Unknown";
        Decoded.exitcode = 0;
        Decoded.payload = "";
        
        // analyse first byte
        unsigned char fByte = data[0];
        
        bool fin = fByte&0x80;
        bool rsv1 = fByte&0x40;
        bool rsv2 = fByte&0x20;
        bool rsv3 = fByte&0x10;
        
        int opcode = 8*(fByte&0x08) + 4*(fByte&0x04) + 2*(fByte&0x02) + (fByte&0x01);
        
        if (opcode == 1) {
            Decoded.type = "text";
        } else if (opcode == 8) {
            Decoded.type = "close";
        } else if (opcode == 9) {
            Decoded.type = "ping";
        } else if (opcode == 10) {
            Decoded.type = "pong";
        } else {
            Decoded.type = "unknown opcode";
            Decoded.exitcode = 1003;
            return Decoded;
        }
        
        //printf("Opcode: %s\n", Decoded.type.c_str());
        
        // analyse second byte
        unsigned char sByte = data[1];
        bool masked = fByte&0x80;
        
        if (!masked) {
            Decoded.exitcode = 1002;
            return Decoded; // unmasked
        }
        
        // payload length
        int payloadLength = sByte & 127;
        
        //printf("Payload Length: %d", payloadLength);
        
        std::string mask;
        int payloadOffset;
        
        if (payloadLength == 126) {
            Decoded.type = "text"; // @TODO
            Decoded.payload = "Error with " + data;
            return Decoded;
            
            mask = data.substr(4, 4);
            payloadOffset = 8;
        }
        else if (payloadLength == 127) {
            Decoded.type = "text"; // @TODO
            Decoded.payload = "Error with " + data;
            return Decoded;
            
            mask = data.substr(10, 4);
            payloadOffset = 14;
        }
        else {
            mask = data.substr(2, 4);
            payloadOffset = 6;
        }
        
        //printf("Mask: %s Offset: %d", mask.c_str(), payloadOffset);
        
        int dataLength = data.length();
        
        std::stringstream sstr;
        for (int i = payloadOffset; i < dataLength; i++) {
            int j = i - payloadOffset;
            unsigned char d = data[i];
            unsigned char m = mask[j % 4];
            unsigned char unmasked = d ^ m;
            
            sstr << unmasked;
        }
        
        Decoded.payload = sstr.str();

        return Decoded;
         */
    }
}
