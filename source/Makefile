ARCH = amd64
ARCHDEFINE = _AMD64_
HOSTPLATFORM = cygwin-pc-i686
TARGETPLATFORM = x86_64-elf


SRCDIR = .
INCDIR = $(SRCDIR)/include
OBJDIR = ./../output/obj
BINDIR = ./../output/bin
ASMDIR = ./../output/asm
DEPDIR = ./../output/dep

STDLIBINCDIR = $(SRCDIR)/lib/include -I$(SRCDIR)/lib/internals
ARCHINCDIR = $(SRCDIR)/arch/$(ARCH)/include
DEFAULTINCDIRS = -I$(STDLIBINCDIR) -I$(ARCHINCDIR) -I$(INCDIR)

TOOLBINDIR = ./../toolchain/$(HOSTPLATFORM)/bin
TOOLPREFIX = $(TOOLBINDIR)/$(TARGETPLATFORM)
CC = $(TOOLPREFIX)-gcc
LD = $(TOOLPREFIX)-ld
AR = $(TOOLPREFIX)-ar
DD = dd
AS = $(TOOLBINDIR)/nasm
RM = rm
CP = cp
MCOPY = $(TOOLBINDIR)/mcopy
RANLIB = $(TOOLPREFIX)-ranlib

include boot/Makefile
include kernel/Makefile
include lib/Makefile

.PHONY : image clean-image

image : $(BINDIR)/wnos-hdd.img
clean-image : 
	$(RM) -f $(BINDIR)/wnos-hdd.img

$(BINDIR)/wnos-hdd.img : $(BINDIR)/kernel.elf64 $(BINDIR)/fat32mbr.bin $(BINDIR)/ezloader.bin
	@$(CP) $(SRCDIR)/template.img $(BINDIR)/wnos-hdd.img
	@echo CP $(BINDIR)/wnos-hdd.img
	@$(MCOPY) -i $(BINDIR)/wnos-hdd.img $(BINDIR)/ezloader.bin ::ezloader.bin
	@echo MCOPY $(BINDIR)/ezloader.bin
	@$(MCOPY) -i $(BINDIR)/wnos-hdd.img $(BINDIR)/kernel.elf64 ::kernel.elf64
	@echo MCOPY $(BINDIR)/kernel.elf64
	@$(DD) if=$(BINDIR)/wnos-hdd.img ibs=8192k of=$(BINDIR)/wnos-hdd-padded.img conv=sync 2> /dev/null
	@echo DD $(BINDIR)/wnos-hdd-padded.img


.PHONY : all clean
all : kernel boot lib image

clean : clean-boot clean-kernel clean-lib clean-image
