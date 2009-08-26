###############################################################################
### Targets ###

bin_targets = hello

# Hello
hello_sources = src/hello.c
hello_libs = -lpthread

# vpaths - add any directory that you have source in
vpath %.c src

###############################################################################
### Config ###

BINDIR := bin
OBJDIR := obj
DEPDIR := deps

# flags
CFLAGS +=	-O2 \
		-Wall 

CPPFLAGS +=	-Iinclude

###############################################################################
### Main Rule ###

all : $(bin_targets)

###############################################################################
### Implicit Rules ###
$(DEPDIR) $(BINDIR) $(OBJDIR) $(DEPDIR)/$(BINDIR):
	@mkdir -p $@

$(bin_targets) : % : $(BINDIR) $(DEPDIR)/$(BINDIR)/%.d $(BINDIR)/% ;

$(DEPDIR)/$(BINDIR)/%.d : | $(DEPDIR)/$(BINDIR)
	@set -e; rm -f $@;\
	echo '$(BINDIR)/$* : $(addprefix $(OBJDIR)/,$(notdir $($(join \
	$*,_sources):.c=.o))) $(addprefix $(DEPDIR)/,$($(join \
	$*,_sources):.c=.d)) $(BINDIR)' > $@; \
	echo '	$$(CC) $$(LDFLAGS) $(addprefix $(OBJDIR)/,$(notdir $($(join \
	$*,_sources):.c=.o))) $($(join $*,_libs)) -o $$@' >> $@

$(OBJDIR)/%.o : %.c | $(OBJDIR)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

# include binary target rules
BINDEPS=$(addsuffix .d,$(addprefix $(DEPDIR)/$(BINDIR)/,$(bin_targets)))
sinclude $(BINDEPS)

$(DEPDIR)/%.d : %.[cS] | $(DEPDIR)
	@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< | \
	sed 's,\($(*F)\).o[ :]*,$(OBJDIR)/\1.o $@ : ,g' > $@

# include auto dependency rules
SRCDEPS=$(foreach target,$(bin_targets),$(addprefix $(DEPDIR)/,$($(join \
	$(target),_sources):.c=.d)))
sinclude $(SRCDEPS)

###############################################################################
### Clean Up ###
.PHONY : clean
clean:
	rm -rf $(BINDIR) $(OBJDIR) $(DEPDIR)
