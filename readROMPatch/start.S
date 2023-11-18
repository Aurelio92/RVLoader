.set r0,0;      .set r1,1;      .set r2,2;      .set r3,3;      .set r4,4;
.set r5,5;      .set r6,6;      .set r7,7;      .set r8,8;      .set r9,9;
.set r10,10;    .set r11,11;    .set r12,12;    .set r13,13;    .set r14,14;
.set r15,15;    .set r16,16;    .set r17,17;    .set r18,18;    .set r19,19;
.set r20,20;    .set r21,21;    .set r22,22;    .set r23,23;    .set r24,24;
.set r25,25;    .set r26,26;    .set r27,27;    .set r30,28;    .set r29,29;
.set r30,30;    .set r31,31;
.set sp,1;

.set SRR0, 26
.set SRR1, 27

.extern _main

.section .init

    .globl _start
_start:
    b       _main

# just some helper functions
    .globl disableIRQs
disableIRQs:
    mfmsr 3
    rlwinm 4,3,0,17,15
    mtmsr 4
    extrwi 3,3,1,16
    blr

    .globl restoreIRQs
restoreIRQs:
    cmpwi 3,0
    mfmsr 4
    beq doRestDisable
    # re-enable interrupts
    ori 5,4,0x8000
    b restoreSetVal
doRestDisable:
    rlwinm 5,4,0,17,15
restoreSetVal:
    mtmsr 5
    extrwi 3,4,1,16
    blr
