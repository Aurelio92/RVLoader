import sys
import os

OPCODE_BLR = 0x4e800020

#Based off Dolphin's HashSignatureDB::ComputeCodeChecksum
def computeNextFunctionChecksum(file):
    curLen = 0
    checkSum = 0

    while True:
        opcodeData = file.read(4)
        if len(opcodeData) != 4:
            return [checkSum, curLen]
        
        opcode = int.from_bytes(opcodeData, byteorder = "big", signed = False)

        if curLen == 0 and opcode == 0: #Skips first null data
            continue

        op = opcode & 0xFC000000
        op2 = 0
        op3 = 0
        auxop = op >> 26

        match auxop:
            case 4:  # PS instructions
                op2 = opcode & 0x0000003F
                match op2:
                    case 0:
                        op3 = opcode & 0x000007C0
                    case 8:
                        op3 = opcode & 0x000007C0
                    case 16:
                        op3 = opcode & 0x000007C0
                    case 21:
                        op3 = opcode & 0x000007C0
                    case 22:
                        op3 = opcode & 0x000007C0

            case 7:  # addi muli etc
                op2 = opcode & 0x03FF0000
            case 8:
                op2 = opcode & 0x03FF0000
            case 10:
                op2 = opcode & 0x03FF0000
            case 11:
                op2 = opcode & 0x03FF0000
            case 12:
                op2 = opcode & 0x03FF0000
            case 13:
                op2 = opcode & 0x03FF0000
            case 14:
                op2 = opcode & 0x03FF0000
            case 15:
                op2 = opcode & 0x03FF0000

            case 19:  # MCRF??
                op2 = opcode & 0x000007FF
            case 31:  # integer
                op2 = opcode & 0x000007FF
            case 63:  # fpu
                op2 = opcode & 0x000007FF

            case 59:  # fpu
                op2 = opcode & 0x0000003F
                if op2 < 16:
                    op3 = opcode & 0x000007C0

            case _:
                if auxop >= 32 and auxop < 56:
                    op2 = opcode & 0x03FF0000
        
        checkSum = (((checkSum << 17) & 0xFFFE0000) | ((checkSum >> 15) & 0x0001FFFF))
        checkSum = checkSum ^ (op | op2 | op3)
        curLen += 4

        if opcode == OPCODE_BLR:
            return [checkSum, curLen]

def findFunctions(file):
    functionAddresses = []
    file.seek(0, os.SEEK_END)
    fileSize = file.tell()
    file.seek(0)
    while True:
        opcodeData = file.read(4)
        if len(opcodeData) != 4:
            return functionAddresses
        
        opcode = int.from_bytes(opcodeData, byteorder = "big", signed = False)
        if (opcode & 0xFC000003) == 0x48000001:
            offset = opcode & 0x03FFFFFC
            if offset & 0x02000000:
                offset = -(((~offset) & 0x01FFFFFF) + 1)
            address = file.tell() - 4 + offset
            if address not in functionAddresses and address < fileSize and address > 0:
                functionAddresses.append(address)

with open(sys.argv[1], "rb") as inFile:
    functionAddresses = findFunctions(inFile)
    for address in functionAddresses:
        inFile.seek(address)
        [checkSum, funcLen] = computeNextFunctionChecksum(inFile)
        if funcLen > 0:
            print("{:08X} {:08X} {:08X}".format(address, checkSum, funcLen))