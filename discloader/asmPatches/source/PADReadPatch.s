#include <asm.h>

HUDCaller:
    stwu sp, -0x20(sp)
    mflr r0
    stw r0, 0x24(sp)
    stw r3, 0x18(sp)
callOSDisableInterrupts:
    bl 0 # Placeholder for OSDisableInterrupts_Address
    stw r3, 0x1C(sp)
setIOSMemIBATAddress:
    lis r3, 0x0000 # setIOSMemIBAT_MSB
    ori r3, r3, 0x0000 # setIOSMemIBAT_LSB
    bl 0 # Placeholder for runInSupervisorMode
    lwz r3, 0x18(sp)
HUDHandlerAddress:
    lis r12, 0x0000 # HUDHandler_MSB
    ori r12, r12, 0x0000 # HUDHandler_LSB
    mtctr r12
    bctrl
    lwz r3, 0x1C(sp)
    lwz r0, 0x24(sp)
    mtlr r0
    addi sp, sp, 0x20
    blr

.long callOSDisableInterrupts
.long setIOSMemIBATAddress
.long HUDHandlerAddress
