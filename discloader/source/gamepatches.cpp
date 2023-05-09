#include <gccore.h>
#include <stdio.h>
#include <vector>
#include "gamepatches.h"

#define OPCODE_BLR 0x4e800020

//computeCodeChecksum was taken from dolphin's signature database code
static u32 computeCodeChecksum(u8* data, u32 len) {
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

static u32 findFunctionStart(void* dst, u32 len, u32 offset) {
    u32 opcode = *((u32*)(dst + offset));
    while (offset < len && opcode == 0) {
        offset += 4;
        opcode = *((u32*)(dst + offset));
    }
    return offset;
}

static u32 findFunctionEnd(void* dst, u32 len, u32 offset) {
    u32 opcode = *((u32*)(dst + offset));
    while (offset < len && opcode != OPCODE_BLR) {
        offset += 4;
        opcode = *((u32*)(dst + offset));
    }
    return offset;
}

GamePatch compareChecksum(u32 functionLen, u32 functionChecksum, u32 functionAddress) {
    GamePatch curPatch;

    curPatch.address = functionAddress;
    curPatch.checksum = functionChecksum;
    curPatch.len = functionLen;
    
    if (functionChecksum == __VIRetraceHandler0_checksum && functionLen == __VIRetraceHandler0_len) {
        return curPatch;
    }
    if (functionChecksum == __VIRetraceHandler1_checksum && functionLen == __VIRetraceHandler1_len) {
        return curPatch;
    }
    if (functionChecksum == __VIRetraceHandler2_checksum && functionLen == __VIRetraceHandler2_len) {
        return curPatch;
    }
    if (functionChecksum == __VIRetraceHandler3_checksum && functionLen == __VIRetraceHandler3_len) {
        return curPatch;
    }

    if (functionChecksum == SISetXY0_checksum && functionLen == SISetXY0_len) {
        return curPatch;
    }
    if (functionChecksum == SISetXY1_checksum && functionLen == SISetXY1_len) {
        return curPatch;
    }

    if (functionChecksum == ReadFont0_checksum && functionLen == ReadFont0_len) {
        return curPatch;
    }
    if (functionChecksum == ReadFont1_checksum && functionLen == ReadFont1_len) {
        return curPatch;
    }

    if (functionChecksum == PADRead0_checksum && functionLen == PADRead0_len) {
        return curPatch;
    }
    if (functionChecksum == PADRead1_checksum && functionLen == PADRead1_len) { //RMCP
        return curPatch;
    }

    if (functionChecksum == VISetNextFrameBuffer_checksum && functionLen == VISetNextFrameBuffer_len) {
        return curPatch;
    }

    curPatch.address = 0;
    curPatch.checksum = 0;
    curPatch.len = 0;

    return curPatch;
}

void findPatches(void* dst, u32 len, std::vector<GamePatch>& patches) {
    u32 functionStart = 0;
    u32 functionEnd;
    u32 functionLen;
    u32 functionChecksum;
    GamePatch curPatch;

    do {
        functionEnd = findFunctionEnd(dst, len, functionStart);
        functionLen = functionEnd - functionStart + 4;
        functionChecksum = computeCodeChecksum((u8*)(dst + functionStart), functionLen);
        curPatch = compareChecksum(functionLen, functionChecksum, (u32)(dst + functionStart));
        if (curPatch.address) {
            patches.push_back(curPatch);
        }
        functionStart = findFunctionStart(dst, len, functionEnd + 4);
    } while (functionEnd < len);
}