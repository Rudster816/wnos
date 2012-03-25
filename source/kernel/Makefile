.PHONY : kernel clean-kernel

kernel : $(BINDIR)/kernel.elf64
clean-kernel :
	rm -f $(BINDIR)/kernel.elf64
	rm -f $(OBJDIR)/kernel/*.o
	rm -f $(OBJDIR)/kernel/*/*.o
	rm -f $(ASMDIR)/kernel/*.s
	rm -f $(ASMDIR)/kernel/*/*.s
	rm -f $(DEPDIR)/kernel/*.d
	rm -f $(DEPDIR)/kernel/*/*.d

KERNEL_WARNINGS = -Wall -Wextra
KERNEL_CCFLAGS = -O2 -g $(KERNEL_WARNINGS) -m64 -std=c99 $(DEFAULTINCDIRS) -nostdlib -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow

KERNEL_SRCROOT = $(SRCDIR)/kernel
KERNEL_SRCSUBS = mm
KERNEL_SRCDIRS = $(KERNEL_SRCROOT) $(foreach SUBDIR,$(KERNEL_SRCSUBS),$(KERNEL_SRCROOT)/$(SUBDIR))

KERNEL_CC_OBJS = $(patsubst %.o, $(OBJDIR)/kernel/%.o, $(patsubst %.c, %.o, $(notdir $(wildcard $(SRCDIR)/kernel/*.c))))
KERNEL_CC_OBJS += $(foreach SUBDIR,$(KERNEL_SRCSUBS),$(patsubst %.o, $(OBJDIR)/kernel/$(SUBDIR)/%.o, $(patsubst %.c, %.o, $(notdir $(wildcard $(SRCDIR)/kernel/$(SUBDIR)/*.c)))))

KERNEL_AS_OBJS = $(patsubst %.o, $(OBJDIR)/kernel/%.o, $(patsubst %.asm, %.o, $(notdir $(wildcard $(SRCDIR)/kernel/*.asm))))
KERNEL_AS_OBJS += $(foreach SUBDIR,$(KERNEL_SRCSUBS),$(patsubst %.o, $(OBJDIR)/kernel/$(SUBDIR)/%.o, $(patsubst %.asm, %.o, $(notdir $(wildcard $(SRCDIR)/kernel/$(SUBDIR)/*.asm)))))

KERNEL_DEPS = $(patsubst %.d, $(DEPDIR)/kernel/%.d, $(patsubst %.c, %.d, $(notdir $(wildcard $(SRCDIR)/kernel/*.c))))
KERNEL_DEPS += $(foreach SUBDIR,$(KERNEL_SRCSUBS),$(patsubst %.d, $(DEPDIR)/kernel/$(SUBDIR)/%.d, $(patsubst %.c, %.d, $(notdir $(wildcard $(SRCDIR)/kernel/$(SUBDIR)/*.c)))))

$(KERNEL_DEPS) : $(DEPDIR)/%.d : $(SRCDIR)/%.c
	@$(CC) $(KERNEL_CCFLAGS) -c $< -MM -MT $(OBJDIR)/$(patsubst %.c,%.o, $<) -o $@
	@echo CC $@

-include $(KERNEL_DEPS)

$(OBJDIR)/kernel/bootstrap.o : $(BINDIR)/trampoline.bin

$(KERNEL_CC_OBJS): $(OBJDIR)/%.o: %.c
	@$(CC) $(KERNEL_CCFLAGS) -c $< -o $@ -Wa,-aln=$(ASMDIR)/$(patsubst %.c,%.s, $<)
	@echo CC $@

$(KERNEL_AS_OBJS): $(OBJDIR)/%.o: $(SRCDIR)/%.asm
	@$(AS) -f elf64 -o $@ $<
	@echo NASM $@

$(BINDIR)/kernel.elf64 : $(KERNEL_AS_OBJS) $(KERNEL_CC_OBJS) $(OBJDIR)/pdclib.a $(SRCDIR)/kernel/linker.ld
	@$(LD) -T $(SRCDIR)/kernel/linker.ld -z max-page-size=0x1000 -o $(BINDIR)/kernel.elf64 $(KERNEL_AS_OBJS) $(KERNEL_CC_OBJS) $(OBJDIR)/pdclib.a
	@echo LD $@