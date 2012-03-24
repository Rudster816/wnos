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

TOOLBINDIR = ./../toolchain/$(HOSTPLATFORM)/$(TARGETPLATFORM)/bin
TOOLPREFIX = $(TOOLBINDIR)/$(TARGETPLATFORM)
CC = $(TOOLPREFIX)-gcc
LD = $(TOOLPREFIX)-ld
AR = $(TOOLPREFIX)-ar
RANLIB = $(TOOLPREFIX)-ranlib
AS = $(TOOLBINDIR)/nasm
RM = rm
CP = cp
MCOPY = $(TOOLBINDIR)/mcopy

include boot/Makefile
include kernel/Makefile
include lib/Makefile

.PHONY : image clean-image

image : $(BINDIR)/wnos-hdd.img
clean-image : 
	$(RM) -f $(BINDIR)/wnos-hdd.img

$(BINDIR)/wnos-hdd.img : $(BINDIR)/kernel.elf64 $(BINDIR)/fat32mbr.bin $(BINDIR)/ezloader.bin
	$(CP) $(SRCDIR)/template.img $(BINDIR)/wnos-hdd.img
	$(MCOPY) -i $(BINDIR)/wnos-hdd.img $(BINDIR)/ezloader.bin ::ezloader.bin
	$(MCOPY) -i $(BINDIR)/wnos-hdd.img $(BINDIR)/kernel.elf64 ::kernel.elf64


.PHONY : all clean
all : kernel boot lib image

clean : clean-boot clean-kernel clean-lib clean-image
