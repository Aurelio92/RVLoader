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

all: $(BINFILES)

-include $(DFILES)
.PRECIOUS: %.elf %.bin

%.elf: $(SOURCEDIR)/%.s
	@mkdir -p $(BUIILDDIR)
	@echo " ASSEMBLE    $<"
	@$(AS) $< -mregnames -MD $(BUIILDDIR)/$(@:.elf=.d) -o $(BUIILDDIR)/$@

%.bin: %.elf
	@mkdir -p $(OUTPUTDIR)
	@echo " STRIP       $<"
	@$(STRIP) $(BUIILDDIR)/$< -O binary -S -s -o $(OUTPUTDIR)/$@

clean:
	@echo clean ...
	@rm -fr $(BUIILDDIR)/*.d $(OUTPUTDIR)/*.bin $(BUIILDDIR)/*.elf
