PREFIX = $(DEVKITPPC)/bin/powerpc-eabi-

AR = $(PREFIX)ar
AS = $(PREFIX)as
CC = $(PREFIX)gcc
CXX = $(PREFIX)g++
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy
RANLIB = $(PREFIX)ranlib
STRIP = $(PREFIX)strip

SOURCEDIR	:=	source
BUIILDDIR	:=	build
OUTPUTDIR	:=	out

SFILES		:=	$(notdir $(wildcard $(SOURCEDIR)/*.s))
DFILES		:=	$(SFILES:.s=.d)
ELFFILES	:=	$(SFILES:.s=.elf)
BINFILES	:=	$(ELFFILES:.elf=.bin)

TARGET		:=	$(foreach file,$(BINFILES),$(OUTPUTDIR)/$(file))

all: $(TARGET)

-include $(DFILES)
.PRECIOUS: %.elf %.bin

$(OUTPUTDIR)/%.bin: $(SOURCEDIR)/%.s
	@mkdir -p $(OUTPUTDIR)
	@mkdir -p $(BUIILDDIR)
	@echo " ASSEMBLE    $<"
	@$(AS) $< -mregnames -MD $(BUIILDDIR)/$(notdir $(@:.bin=.d)) -o $(BUIILDDIR)/$(notdir $(@:.bin=.elf))
	@echo " STRIP       $(notdir $(@:.bin=.elf))"
	@$(STRIP) $(BUIILDDIR)/$(notdir $(@:.bin=.elf)) -O binary -S -s -o $(OUTPUTDIR)/$(notdir $@)

clean:
	@echo clean ...
	@rm -fr $(BUIILDDIR)/*.d $(OUTPUTDIR)/*.bin $(BUIILDDIR)/*.elf
