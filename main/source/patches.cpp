#include <gccore.h>
#include <stdio.h>
#include <string.h>
#include <set>
#include "patches.h"

#include "readROM_bin.h"

#define BLR_OPCODE  0x4e800020

#define BL_OPCODE   0x48000001
#define BL_MASK     0xFC000003
#define BL_OFFSET   0x03FFFFFC
#define BL_SIGN     0x02000000
#define BL_SIGN_EXT 0xFC000000

//Based off Dolphin's HashSignatureDB::ComputeCodeChecksum
u32 computeCodeChecksum(u8* data, u32 len) {
    u32 sum = 0;
    u32 i;
    for (i = 0; i < len; i += 4) {
        u32 opcode = *((u32*)(&data[i]));
        u32 op = opcode & 0xFC000000;
        u32 op2 = 0;
        u32 op3 = 0;
        u32 auxop = op >> 26;
        switch (auxop) {
            case 4:  // PS instructions
                op2 = opcode & 0x0000003F;
                switch (op2) {
                    case 0:
                    case 8:
                    case 16:
                    case 21:
                    case 22:
                        op3 = opcode & 0x000007C0;
                } break;

            case 7:  // addi muli etc
            case 8:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
                op2 = opcode & 0x03FF0000;
            break;

            case 19:  // MCRF??
            case 31:  // integer
            case 63:  // fpu
                op2 = opcode & 0x000007FF;
            break;
            case 59:  // fpu
                op2 = opcode & 0x0000003F;
                if (op2 < 16)
                    op3 = opcode & 0x000007C0;
            break;
            default:
                if (auxop >= 32 && auxop < 56)
                    op2 = opcode & 0x03FF0000;
            break;
        }
        // Checksum only uses opcode, not opcode data, because opcode data changes
        // in all compilations, but opcodes don't!
        sum = (((sum << 17) & 0xFFFE0000) | ((sum >> 15) & 0x0001FFFF));
        sum = sum ^ (op | op2 | op3);
    }
    return sum;
}

void listFunctionsInDOL(dolhdr* dol, std::vector<DOLFunction>& functions) {
    std::set<u32> functionsSet; //Useful to make sure we don't have a function listed already

    for (u32 i = 0; i < 7; i++) {
        u32 textOffset = dol->offsetText[i];
        u32* textData = (u32*)(((u8*)dol) + textOffset);
        u32 textLen = dol->sizeText[i];
        u32 textAddress = dol->addressText[i];

        for (u32 j = 0; j < textLen >> 2; j++) {
            u32 opcode = textData[j];

            if ((opcode & BL_MASK) == BL_OPCODE) {
                DOLFunction function;
                u32 offset = opcode & BL_OFFSET;

                if (offset & BL_SIGN) {
                    offset |= BL_SIGN_EXT;
                }
                
                function.address = textAddress + (j << 2) + offset;
                function.offset = textOffset + (j << 2) + offset;

                //Add functions to map
                if (functionsSet.find(function.address) == functionsSet.end()) {
                    function.len = 4;
                    u32* functionData = &textData[(function.address - textAddress) >> 2];

                    while (*functionData != BLR_OPCODE) {
                        functionData++;
                        function.len += 4;
                    }
                    functionsSet.insert(function.address);
                    function.checksum = computeCodeChecksum((u8*)&textData[(function.address - textAddress) >> 2], function.len);
                    functions.push_back(function);
                    //printf("%08X %08X %08X %08X\n", function.address, function.offset, function.len, function.checksum);
                }
            }
        }
    }
}

void injectMXPatch(u8* oldReadROM) {
    *((vu32*)(&oldReadROM[0x00])) = 0x9421ffe0; //stwu sp, -0x20(sp)
    *((vu32*)(&oldReadROM[0x04])) = 0x7c0802a6; //mflr r0
    *((vu32*)(&oldReadROM[0x08])) = 0x90010024; //stw r0, 0x24(sp)
    *((vu32*)(&oldReadROM[0x0C])) = 0x3d809364; //lis r12, readROM_MSB
    *((vu32*)(&oldReadROM[0x10])) = 0x618c0000; //ori r12, r12, readROM_LSB
    *((vu32*)(&oldReadROM[0x14])) = 0x7d8903a6; //mtctr r12
    *((vu32*)(&oldReadROM[0x18])) = 0x4e800421; //bctrl
    *((vu32*)(&oldReadROM[0x1C])) = 0x80010024; //lwz r0, 0x24(sp)
    *((vu32*)(&oldReadROM[0x20])) = 0x7c0803a6; //mtlr r0
    *((vu32*)(&oldReadROM[0x24])) = 0x38210020; //addi sp, sp, 0x20
    *((vu32*)(&oldReadROM[0x28])) = 0x4e800020; //blr
    DCFlushRange(oldReadROM, 0x2C);

    memcpy((void*)0x93640000, readROM_bin, readROM_bin_size);
    DCFlushRange((void*)0x93640000, readROM_bin_size);
}