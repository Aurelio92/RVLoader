.set r0,0;      .set r1,1;      .set r2,2;      .set r3,3;      .set r4,4;
.set r5,5;      .set r6,6;      .set r7,7;      .set r8,8;      .set r9,9;
.set r10,10;    .set r11,11;    .set r12,12;    .set r13,13;    .set r14,14;
.set r15,15;    .set r16,16;    .set r17,17;    .set r18,18;    .set r19,19;
.set r20,20;    .set r21,21;    .set r22,22;    .set r23,23;    .set r24,24;
.set r25,25;    .set r26,26;    .set r27,27;    .set r28,28;    .set r29,29;
.set r30,30;    .set r31,31;
.set sp,1;

.extern _main
.extern _memset

.section .init

    .globl _start
_start:
    mfmsr   r3
    rlwinm  r3,r3,0x0,0x11,0xf
    ori     r3,r3,0x2000
    isync

    lis     r1, 0x9301
    subi    r1, r1, 0x7c80
    li      r0, 0x00
    stwu    r0, -0x40(r1)
    lis     r3, 0x9300
    addi    r3, r3, 0x0368
    li      r4, 0x00
    lis     r5, 0x9300
    addi    r5, r5, 0x0368
    subf    r5, r3, r5
    bl      _memset
    b       _main
