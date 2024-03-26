#---------------------------------------------------------------------------------
.SUFFIXES:
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif
#---------------------------------------------------------------------------------
TARGET	:= codehandler.bin codehandleronly.bin
#---------------------------------------------------------------------------------
PREFIX = $(DEVKITPPC)/bin/powerpc-eabi-

AR = $(PREFIX)ar
AS = $(PREFIX)as
CC = $(PREFIX)gcc
CXX = $(PREFIX)g++
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
RANLIB = $(PREFIX)ranlib
STRIP = $(PREFIX)strip
#---------------------------------------------------------------------------------
CFLAGS		:= -nostartfiles -nodefaultlibs -Wl,-Ttext,0x80001900
STRIPFLAGS	:= --strip-debug --strip-all --discard-all -F elf32-powerpc
OCFLAGS		:= -I elf32-powerpc -O binary
#---------------------------------------------------------------------------------
.PHONY: all clean

all: $(TARGET)

%.bin : %.s
	@echo " COMPILE     $<"
	@$(CC) $(CFLAGS) -o $@ $<
	@$(OBJDUMP) -S $@ > $(@:.bin=.asm)
	@echo " STRIP       $<"
	@$(STRIP) $(STRIPFLAGS) $@
	@echo " OBJCOPY     $<"
	@$(OBJCOPY) $(OCFLAGS) $@
	@cp $@ ../driveRoot/rvloader/Hiidra/
clean:
	-$(RM) $(TARGET) $(TARGET:.h=.bin)
