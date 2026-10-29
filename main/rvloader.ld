/*--------------------------------------------------------------------------------
	devkitPPC's rvl.ld plus a MEM2 segment. Passing -T to gcc replaces the stock one
	(INSERT can't be used: it only works with ld's built-in script).
	The MEM2 rules come before libogc_common.ld's, so they match these libraries' input
	sections before its *(.text*) does. ld adds long branch stubs for calls between MEM1 and MEM2.
--------------------------------------------------------------------------------*/

PHDRS {
	stub PT_LOAD FLAGS(5);
	mem2 PT_LOAD FLAGS(5);
}

SECTIONS {

	/* stub is loaded at physical address 0x00003400 (though both 0x80003400 and 0x00003400 are equivalent for IOS) */
	. = 0x00003400;

	.stub : {
		KEEP(*(.stub))
	} :stub = 0

	/* The biggest libraries' code and read-only data, so MEM1 is left for the buffers the GPU needs */
	.mem2text 0x90002000 : {
		*liblua.a:*(.text .text.*)
		*libfreetype.a:*(.text .text.*)
		*libstdc++.a:*(.text .text.*)
	} :mem2

	.mem2rodata : {
		*liblua.a:*(.rodata .rodata.*)
		*libfreetype.a:*(.rodata .rodata.*)
		*libstdc++.a:*(.rodata .rodata.*)
		/* Files embedded from data/, only read by the CPU */
		*discloader.dol.o(.rodata .rodata.*)
		*poppins.ttf.o(.rodata .rodata.*)
		*wiitdb.txt.bz2.o(.rodata .rodata.*)
		. = ALIGN(32);
		__mem2_end = .;
	} :mem2

	/* Base address */
	. = 0x81330000;

}

PROVIDE_HIDDEN( __Arena2Lo    = 0x90002000 );
PROVIDE_HIDDEN( __Arena2Hi    = 0x933e0000 );
PROVIDE_HIDDEN( __ipcbufferLo = 0x933e0000 );
PROVIDE_HIDDEN( __ipcbufferHi = 0x93400000 );

INCLUDE libogc_common.ld
