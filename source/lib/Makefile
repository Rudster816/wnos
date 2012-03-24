.PHONY : lib clean-lib
lib : $(OBJDIR)/pdclib.a
clean-lib : 
	$(RM) -f $(OBJDIR)/pdclib.a
	$(RM) -f $(OBJDIR)/lib/*/*.o
	$(RM) -f $(ASMDIR)/lib/*/*.s
	$(RM) -f $(DEPDIR)/lib/*/*.d

LIB_SRCROOT = $(SRCDIR)/lib
LIB_SRCSUBS = _PDCLIB ctype inttypes locale signal stdio stdlib string
LIB_SRCDIRS = $(LIB_SRCROOT) $(foreach SUBDIR,$(LIB_SRCSUBS),$(LIB_SRCROOT)/$(SUBDIR))

LIB_OBJS = $(patsubst %.o, $(OBJDIR)/lib/%.o, $(patsubst %.c, %.o, $(notdir $(wildcard $(SRCDIR)/lib/*.c))))
LIB_OBJS += $(foreach SUBDIR,$(LIB_SRCSUBS),$(patsubst %.o, $(OBJDIR)/lib/$(SUBDIR)/%.o, $(patsubst %.c, %.o, $(notdir $(wildcard $(SRCDIR)/lib/$(SUBDIR)/*.c)))))

LIB_DEPS = $(patsubst %.d, $(DEPDIR)/lib/%.d, $(patsubst %.c, %.d, $(notdir $(wildcard $(SRCDIR)/lib/*.c))))
LIB_DEPS += $(foreach SUBDIR,$(LIB_SRCSUBS),$(patsubst %.d, $(DEPDIR)/lib/$(SUBDIR)/%.d, $(patsubst %.c, %.d, $(notdir $(wildcard $(SRCDIR)/lib/$(SUBDIR)/*.c)))))
	
LIB_WARNINGS := -Wall -Wextra -pedantic -Wno-unused-parameter -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Winline -Wno-long-long -Wuninitialized -Wstrict-prototypes 
LIB_CCFLAGS := -O2 -fno-builtin -mcmodel=large -g -std=c99 -I$(STDLIBINCDIR) $(WARNINGS) $(USERFLAGS)

$(OBJDIR)/pdclib.a: $(LIB_OBJS)
	$(AR) rcs $(OBJDIR)/pdclib.a $?
	$(RANLIB) $(OBJDIR)/pdclib.a

$(LIB_DEPS) : $(DEPDIR)/%.d : $(SRCDIR)/%.c
	$(CC) $(LIB_CCFLAGS) -c $< -MM -MT $(OBJDIR)/$(patsubst %.c,%.o, $<) -o $@

-include $(LIB_DEPS)
	
$(LIB_OBJS) : $(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(LIB_CCFLAGS) -c $< -o $@ -Wa,-aln=$(ASMDIR)/$(patsubst %.c,%.s, $<)
