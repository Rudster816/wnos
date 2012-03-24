
.PHONY : boot ezloader fat32mbr trampoline clean-boot clean-ezloader clean-fat32mbr clean-trampoline

boot : ezloader fat32mbr
ezloader: $(BINDIR)/ezloader.bin
fat32mbr: $(BINDIR)/fat32mbr.bin
trampoline: $(BINDIR)/trampoline.bin

$(BINDIR)/fat32mbr.bin : $(SRCDIR)/boot/fat32mbr.asm
	$(AS) -f bin -o $@ $<

$(BINDIR)/trampoline.bin : $(SRCDIR)/boot/trampoline.asm
	$(AS) -f bin -o $@ $<

$(BINDIR)/ezloader.bin : $(SRCDIR)/boot/elf.inc $(SRCDIR)/boot/elfloader.asm $(SRCDIR)/boot/fat32driver.asm $(SRCDIR)/boot/ezloader.asm $(SRCDIR)/boot/multiboot.inc
	$(AS) -f bin -o $@ $(SRCDIR)/boot/ezloader.asm -i$(SRCDIR)/boot/


clean-boot : clean-ezloader clean-fat32mbr clean-trampoline
clean-ezloader :
	$(RM) -f $(BINDIR)/ezloader.bin
clean-fat32mbr :
	$(RM) -f $(BINDIR)/fat32mbr.bin
clean-trampoline : 
	$(RM) -f $(BINDIR)/trampoline.bin
