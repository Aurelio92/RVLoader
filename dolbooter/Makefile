.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

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

MACHDEP = -mcpu=750 -mno-eabi -mhard-float
CFLAGS = $(MACHDEP) -O1 -Wall -pipe -ffunction-sections -finline-functions-called-once -mno-sdata
LDFLAGS = $(MACHDEP) -n -nostartfiles -nostdlib -Wl,-gc-sections
ASFLAGS =

LDSCRIPT = linker.ld

TARGET = dolbooter.elf
TARGET_BIN = dolbooter.bin
OBJS = start.o utils.o main.o

ifeq ($(NOMAPFILE),)
LDFLAGS += -Wl,-Map,$(TARGET).map
endif

ifneq ($(LDSCRIPT),)
LDFLAGS += -Wl,-T$(LDSCRIPT)
endif

DEPDIR = .deps

LIBS =

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "  LINK      $@"
	@$(CC) -g -o $@ $(LDFLAGS) $(OBJS) $(LIBS)
	@$(OBJDUMP) -D $(TARGET) > dis.txt

ifneq ($(LDSCRIPT),)
$(TARGET): $(LDSCRIPT)
endif

%.o: %.s
	@echo " ASSEMBLE    $<"
	@$(CC) $(CFLAGS) $(DEFINES) $(ASFLAGS) -c $< -o $@

%.o: %.S
	@echo " ASSEMBLE    $<"
	@$(CC) $(CFLAGS) $(DEFINES) $(ASFLAGS) -c $< -o $@

%.o: %.c
	@echo " COMPILE     $<"
	@$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

clean:
	rm -rf $(DEPDIR)
	rm -f $(TARGET) $(TARGET).map $(OBJS)

define bin2o
	@echo "  BIN2S     $(notdir $<)"
	@$(BIN2S) -a 32 $< | $(AS) -o $(@)
	@echo "extern const u8" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_end[];" > `(echo $(<F) | tr . _)`.h
	@echo "extern const u8" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"[];" >> `(echo $(<F) | tr . _)`.h
	@echo "extern const u32" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_size";" >> `(echo $(<F) | tr . _)`.h
endef

-include $(DEPDIR)/*

.PHONY: clean

all: $(TARGET_BIN)

$(TARGET_BIN): $(TARGET)
	@echo  "  OBJCPY    $@"
	@$(OBJCOPY) -O binary $< $@
	@cp $@ ../main/data/
	@cp $@ ../bootloader/data/

clean: myclean

myclean:
	-rm -f $(TARGET_BIN)
