.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

SUBPROJECTS := dolbooter bootloader installer discloader codehandler libGUI main rvlbooter copy
.PHONY: all forced clean $(SUBPROJECTS)

all: rvlbooter
forced: clean all

dolbooter:
	@echo " "
	@echo "Building RVLoader dolbooter"
	@echo " "
	$(MAKE) -C dolbooter

bootloader: dolbooter
	@echo " "
	@echo "Building RVLoader bootloader"
	@echo " "
	$(MAKE) -C bootloader

installer: bootloader
	@echo " "
	@echo "Building RVLoader installer"
	@echo " "
	$(MAKE) -C installer

discloader:
	@echo " "
	@echo "Building RVLoader discloader"
	@echo " "
	$(MAKE) -C discloader

codehandler:
	@echo " "
	@echo "Building RVLoader codehandler"
	@echo " "
	$(MAKE) -C codehandler

libGUI:
	@echo " "
	@echo "Building RVLoader libGUI"
	@echo " "
	$(MAKE) -C libGUI install

main: dolbooter bootloader installer discloader codehandler
	@echo " "
	@echo "Building RVLoader main"
	@echo " "
	$(MAKE) -C main

rvlbooter: main
	@echo " "
	@echo "Building RVLoader rvlbooter"
	@echo " "
	$(MAKE) -C rvlbooter

copy: main
	@echo " "
	@echo "Copying build to external drive"
	@echo " "
	@cp main/boot.dol /media/aurelio/SANDISK/apps/rvloader
	@udisksctl unmount -b /dev/sdb
	@udisksctl power-off -b /dev/sdb

clean:
	@echo " "
	@echo "Cleaning all subprojects..."
	@echo " "
	$(MAKE) -C dolbooter clean
	$(MAKE) -C bootloader clean
	$(MAKE) -C installer clean
	$(MAKE) -C discloader clean
	$(MAKE) -C codehandler clean
	$(MAKE) -C libGUI clean
	$(MAKE) -C main clean
	$(MAKE) -C rvlbooter clean