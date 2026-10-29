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

#define BCTR_OPCODE 0x4e800420
#define RFI_OPCODE  0x4c000064
#define TRAP_OPCODE 0x7fe00008
#define B_MASK      0xFC000003
#define B_OPCODE    0x48000000
#define BC_OPCODE   0x40000000
#define BC_OFFSET   0x0000FFFC
#define BC_SIGN     0x00008000
#define BC_SIGN_EXT 0xFFFF0000
#define BC_ALWAYS   0x02800000 //BO = 1z1zz: branch always

#define B_OPCODE    0x48000000
#define B_MASK      0xFC000003
#define B_OFFSET    0x03FFFFFC
#define B_SIGN      0x02000000
#define B_SIGN_EXT  0xFC000000

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

//Returns the byte offset from code[0] a relative b/bc jumps to, or 0xFFFFFFFF if opcode is not one
static u32 branchTarget(u32 opcode, u32 pc) {
    u32 target;
    if ((opcode & B_MASK) == B_OPCODE) {
        target = pc + (opcode & BL_OFFSET);
        if (opcode & BL_SIGN)
            target |= BL_SIGN_EXT;
    } else if ((opcode & B_MASK) == BC_OPCODE) {
        target = pc + (opcode & BC_OFFSET);
        if (opcode & BC_SIGN)
            target |= BC_SIGN_EXT;
    } else {
        return 0xFFFFFFFF;
    }
    return target;
}

//Returns the last instruction of the function (blr, bctr, rfi, trap or a b that leaves no code reachable after it).
//Without one (e.g. ending in a call to a noreturn function) the function spans all maxLen bytes; NULL only if maxLen is 0. Cold blocks GCC places after the blr are included,
//since an earlier forward branch keeps the function open until past them.
//maxLen must stop at the next known function start: branches past it are treated as tail calls.
u32* findFunctionEnd(u32* functionStart, u32* functionLen, u32 maxLen) {
    if (!functionStart || !functionLen) {
        return NULL;
    }

    u32 farthestTarget = 0; //Farthest forward branch target seen so far, as a byte offset from functionStart
    *functionLen = 0;

    for (u32 pc = 0; pc < maxLen; pc += 4) {
        u32 opcode = functionStart[pc >> 2];
        u32 target = branchTarget(opcode, pc); //b/bc without link or absolute addressing
        bool unconditional = (opcode & B_MASK) == B_OPCODE ||
            ((opcode & B_MASK) == BC_OPCODE && (opcode & BC_ALWAYS) == BC_ALWAYS);

        //Targets before the start or past maxLen are tail calls or other functions (u32 wraps negatives)
        bool internalForward = target > pc && target < maxLen;

        //Nothing seen so far jumps past this point, so the code after a jump out is not part of the function
        bool terminator = unconditional || opcode == BLR_OPCODE || opcode == BCTR_OPCODE ||
            opcode == RFI_OPCODE || opcode == TRAP_OPCODE;
        if (terminator && !internalForward && farthestTarget <= pc) {
            *functionLen = pc + 4;
            return &functionStart[pc >> 2];
        }

        if (internalForward && target > farthestTarget)
            farthestTarget = target;
    }

    if (maxLen < 4)
        return NULL;
    *functionLen = maxLen & ~3;
    return &functionStart[(*functionLen >> 2) - 1];
}

void listFunctionsInDOL(dolhdr dol, std::vector<DOLFunction>& functions) {
    std::set<u32> functionsSet; //Every bl target, sorted, so each function can be bounded by the next one

    for (u32 i = 0; i < 7; i++) {
        u32* textData = (u32*)(dol.addressText[i]);
        u32 textAddress = dol.addressText[i];

        for (u32 j = 0; j < dol.sizeText[i] >> 2; j++) {
            u32 opcode = textData[j];

            if ((opcode & BL_MASK) == BL_OPCODE) {
                u32 offset = opcode & BL_OFFSET;

                if (offset & BL_SIGN) {
                    offset |= BL_SIGN_EXT;
                }
                functionsSet.insert(textAddress + (j << 2) + offset);
            }
        }
    }

    for (u32 i = 0; i < 7; i++) {
        u32 textOffset = dol.offsetText[i];
        u32* textData = (u32*)(dol.addressText[i]);
        u32 textLen = dol.sizeText[i];
        u32 textAddress = dol.addressText[i];

        for (auto it = functionsSet.lower_bound(textAddress); it != functionsSet.end() && *it < textAddress + textLen; it++) {
            DOLFunction function;
            function.address = *it;
            function.offset = textOffset + (function.address - textAddress);

            auto next = std::next(it);
            u32 end = (next != functionsSet.end() && *next < textAddress + textLen) ? *next : textAddress + textLen;
            u32* functionData = &textData[(function.address - textAddress) >> 2];

            if (!findFunctionEnd(functionData, &function.len, end - function.address))
                continue;
            function.checksum = computeCodeChecksum((u8*)functionData, function.len);
            functions.push_back(function);
            //printf("%08X %08X %08X %08X\n", function.address, function.offset, function.len, function.checksum);
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
    ICInvalidateRange((void*)0x93640000, readROM_bin_size);
}