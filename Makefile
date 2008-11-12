###############################################################################
### Targets ###

bin_targets = hello

# Hello
hello_sources = src/hello.c
hello_libs = -lpthread

###############################################################################
### Config ###

vpath %.c src

BINDIR = bin
OBJDIR = obj
DEPDIR = deps

# define the tools
PREFIX=
CC=$(PREFIX)gcc
LD=$(PREFIX)ld
AS=$(PREFIX)as
AR=$(PREFIX)ar
OBJCOPY=$(PREFIX)objcopy
SIZE=$(PREFIX)size

# flags
CFLAGS +=	-O2 \
		-Wall 

CPPFLAGS +=	-Iinclude

###############################################################################
### Main Rule ###

all : $(addprefix $(BINDIR)/,$(bin_targets))

###############################################################################
### Implicit Rules ###
$(DEPDIR) $(BINDIR) $(OBJDIR) $(DEPDIR)/$(BINDIR):
	@mkdir -p $@

$(BINDIR)/% :: $(BINDIR) $(DEPDIR)/$(BINDIR)/%.d
	;

$(DEPDIR)/$(BINDIR)/%.d : $(DEPDIR)/$(BINDIR)
	@set -e; rm -f $@;\
	echo '$(BINDIR)/$* :: $(addprefix $(OBJDIR)/,$(notdir $($(join $*,_sources):.c=.o))) $(BINDIR) $(addprefix $(DEPDIR)/,$($(join $*,_sources):.c=.d))' > $@; \
	echo '	$$(CC) $$(LDFLAGS) $(addprefix $(OBJDIR)/,$(notdir $($(join $*,_sources):.c=.o))) $$($(join $*,_libs)) $$(OUTPUT_OPTION)' >> $@

$(OBJDIR)/%.o : %.c $(OBJDIR)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< $(OUTPUT_OPTION)

BINDEPS=$(addsuffix .d,$(addprefix $(DEPDIR)/$(BINDIR)/,$(bin_targets)))
sinclude $(BINDEPS)

$(DEPDIR)/%.d : %.[cS] $(DEPDIR)
	@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($(*F)\).o[ :]*,$(OBJDIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

SRCDEPS=$(foreach target,$(bin_targets),$(addprefix $(DEPDIR)/,$($(join $(target),_sources):.c=.d)))
sinclude $(SRCDEPS)

###############################################################################
### Clean Up ###
.PHONY : clean
clean:
	rm -rf $(BINDIR) $(OBJDIR) $(DEPDIR)
