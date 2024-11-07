.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

PREFIX = $(DEVKITPPC)/bin/powerpc-eabi-

ELF2DOL = $(DEVKITPRO)/tools/bin/elf2dol
BIN2S = $(DEVKITPRO)/tools/bin/bin2s

AR = $(PREFIX)ar
AS = $(PREFIX)as
CC = $(PREFIX)gcc
CXX = $(PREFIX)g++
LD = $(PREFIX)ld
RANLIB = $(PREFIX)ranlib
STRIP = $(PREFIX)strip

MACHDEP = -mcpu=750 -mno-eabi -mhard-float
CFLAGS = $(MACHDEP) -O1 -Wall -pipe -ffunction-sections -finline-functions-called-once -mno-sdata
LDFLAGS = $(MACHDEP) -n -nostartfiles -nostdlib -Wl,-gc-sections
ASFLAGS =

LDSCRIPT = linker.ld

TARGET = dolbooter.elf
TARGET_DOL = dolbooter.dol
OBJS = main.dol.o start.o system_asm.o cache_asm.o main.o

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

ifneq ($(LDSCRIPT),)
$(TARGET): $(LDSCRIPT)
endif

%.dol.o: %.dol
	$(bin2o)

%.o: %.s
	@echo " ASSEMBLE    $<"
	@$(CC) $(CFLAGS) -D_LANGUAGE_ASSEMBLY $(DEFINES) $(ASFLAGS) -c $< -o $@

%.o: %.S
	@echo " ASSEMBLE    $<"
	@$(CC) $(CFLAGS) -D_LANGUAGE_ASSEMBLY $(DEFINES) $(ASFLAGS) -c $< -o $@

%.o: %.c
	@echo " COMPILE     $<"
	@$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

clean:
	rm -rf $(DEPDIR)
	rm -f $(TARGET) $(TARGET).map $(OBJS)

define bin2o
        $(eval _tmpasm := $(shell mktemp))
        @$(BIN2S) -a 32 -H `(echo $(<F) | tr . _)`.h $< > $(_tmpasm)
        @$(CC) -x assembler-with-cpp $(CPPFLAGS) $(ASFLAGS) -c $(_tmpasm) -o $(<F).o
        @rm $(_tmpasm)
endef

-include $(DEPDIR)/*

.PHONY: clean

all: $(TARGET_DOL)

$(TARGET_DOL): $(TARGET)
	@echo  "  ELF2DOL    $@"
	@$(ELF2DOL) $< $@
	@cp $@ boot.dol
	@cp $@ ../driveRoot/apps/RVLoader/boot.dol

clean: myclean

myclean:
	-rm -f $(TARGET_DOL)
