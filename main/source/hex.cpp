#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <gccore.h>
#include "hex.h"

IntelHex::IntelHex() {
    binary = NULL;
    binarySize = 0;
}

IntelHex::IntelHex(const char* filename) {
    u32 extAddress = 0; //Extended address base
    char tempLine[256];

    binarySize = 0;
    binary = NULL;

    FILE* fp = fopen(filename, "r");
    if (!fp)
        return;
    //Compute binary size
    while(!feof(fp)) {
        if (fgets(tempLine, 256, fp) != NULL) {
            if (tempLine[0] == ':') { //Beginning of an intel hex string
                //Check validity of the hex string
                int len = 0;
                while(tempLine[len + 1] != '\r' && tempLine[len + 1] != '\n' && tempLine[len + 1] != '\0') len++;

                //Each hex line must have an even number of hex digits and at least an header + checksum
                /*if ((len & 1) || (len < 10)) {
                    if (binary != NULL) {
                        free(binary);
                        binary = NULL;
                    }
                    binarySize = 0;
                    fclose(fp);
                    return;
                }*/

                u8 byteCount = hex2u8(&tempLine[1]);
                u32 address = hex2u16(&tempLine[3]) + extAddress;
                u8 recordType = hex2u8(&tempLine[7]);
                if (recordType == 0) { //Data
                    //Actual payload
                    if (address >= PAYLOAD_ADDRESS && (address + byteCount) <= PGM_SIZE) {
                        binarySize += byteCount;
                    }
                } else if (recordType == 1) { //End of file
                    break;
                } else if (recordType == 4) { //Extended linear address
                    extAddress = hex2u16(&tempLine[9]) << 16;
                } else { //We found an unsupported record. Error
                    /*if (binary != NULL) {
                        free(binary);
                        binary = NULL;
                    }
                    binarySize = 0;
                    fclose(fp);
                    return;*/
                }
            }
        }
    }

    rewind(fp);
    binarySize = PGM_SIZE - PAYLOAD_ADDRESS;
    //Actually read the binary
    binary = (u8*)malloc(binarySize);
    /*if (binary == NULL) {
        binarySize = 0;
        fclose(fp);
        return;
    }*/
    memset(binary, 0, binarySize);
    extAddress = 0;
    while(!feof(fp)) {
        if (fgets(tempLine, 256, fp) != NULL) {
            if (tempLine[0] == ':') { //Beginning of an intel hex string
                //Check validity of the hex string
                int len = 0;
                while(tempLine[len + 1] != '\r' && tempLine[len + 1] != '\n' && tempLine[len + 1] != '\0') len++;

                //Each hex line must have an even number of hex digits and at least an header + checksum
                if ((len & 1) || (len < 10)) {
                    /*if (binary != NULL) {
                        free(binary);
                        binary = NULL;
                    }
                    binarySize = 0;
                    fclose(fp);
                    return;*/
                }

                u8 byteCount = hex2u8(&tempLine[1]);
                u32 address = hex2u16(&tempLine[3]) + extAddress;
                u8 recordType = hex2u8(&tempLine[7]);
                if (recordType == 0) { //Data
                    //Actual payload
                    if (address >= PAYLOAD_ADDRESS && (address + byteCount) <= PGM_SIZE) {
                        for (int i = 0; i < byteCount; i++) {
                            binary[address + i - PAYLOAD_ADDRESS] = hex2u8(&tempLine[9 + 2 * i]);
                        }
                    }
                } else if (recordType == 1) { //End of file
                    break;
                } else if (recordType == 4) { //Extended linear address
                    extAddress = hex2u16(&tempLine[9]) << 16;
                } else { //We found an unsupported record. Error
                    /*if (binary != NULL) {
                        free(binary);
                        binary = NULL;
                    }
                    binarySize = 0;
                    fclose(fp);
                    return;*/
                }
            }
        }
    }
    fclose(fp);
}

IntelHex::~IntelHex() {
    if (binary != NULL) {
        free(binary);
        binary = NULL;
    }
    binarySize = 0;
}

u8 IntelHex::hex2u8(const char* string) {
    u8 num = 0;
    for (int i = 0; i < 2; i++) {
        num <<= 4;
        char c = string[i];
        if (c == '\0') {
            return num;
        }
        if (c >= '0' && c <= '9') {
            num |= c - '0';
        } else if (c >= 'A' && c <= 'F') {
            num |= c - 'A' + 10;
        } else if (c >= 'a' && c <= 'f') {
            num |= c - 'a' + 10;
        }
    }
    return num;
}

u16 IntelHex::hex2u16(const char* string) {
    u16 num = 0;
    for (int i = 0; i < 4; i++) {
        num <<= 4;
        char c = string[i];
        if (c == '\0') {
            return num;
        }
        if (c >= '0' && c <= '9') {
            num |= c - '0';
        } else if (c >= 'A' && c <= 'F') {
            num |= c - 'A' + 10;
        } else if (c >= 'a' && c <= 'f') {
            num |= c - 'a' + 10;
        }
    }
    return num;
}
