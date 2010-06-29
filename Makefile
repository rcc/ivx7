include buildsystem/func.mk

# Set default flags
CPPFLAGS := -Iinclude
CFLAGS := -Wall -O2
ASFLAGS :=
LDFLAGS :=

# include after default flags so config.mk can append to them
#    first check for command line MKTARGET flag
ifneq ($(MKTARGET),)
include $(MKTARGET)

#    now check for the presence of a linked target.mk
else ifneq ($(strip $(wildcard target.mk)),)
include target.mk

#    build every target in targets/ directory
else
MKTARGETS=$(wildcard targets/*.mk)
endif

#
# Verbose Option
ifeq ($(VERBOSE),1)
	Q :=
else
	Q := @
endif


ifdef MKTARGETS
.PHONY : alltargets
all_targets clean : $(MKTARGETS)
	$(call OUTPUTINFO,DONE,$@)

targets/% : FORCE
	$(call OUTPUTINFO,TARGET,$@)
	$(Q)$(MAKE) MKTARGET=$@ $(MAKECMDGOALS)
	$(Q)echo

FORCE:

else

# Add all the libraries defined in config.mk to LDLIBS
LDLIBS := $(addprefix -l,$(LIBRARIES))

# Add all the frameworks defined in config.mk to LDFLAGS
#   This is only for objective-C in OSX, but it doesn't hurt us here assuming
#   no one defines FRAMEWORKS
LDFLAGS += $(addprefix -framework ,$(FRAMEWORKS))

# Configuration
ifeq ($(words $(CONFIGS)),0)
$(error Must specify at least one config in target makefile (MKTARGET))
endif
ifeq ($(CONFIG),)
CONFIG := $(word 1,$(CONFIGS))
endif
ifeq ($(findstring $(CONFIG),$(CONFIGS)),)
$(error Invalid config specified)
endif
OPTIONS += $($(call TOUPPER,$(CONFIG))_OPTIONS)

# Machine Name and Tool Versions
MACHINE := $(call USCORESUB,$(shell uname -sm))
CCNAME := $(call USCORESUB,$(notdir $(realpath $(shell which $(CC)))))

# Build Directory
BUILDDIR := buildresults/$(TARGET)/$(MACHINE)/$(CCNAME)/$(CONFIG)

# Add in the target name
OPTIONS += __TARGET__='"$(TARGET)"'

# Add in the options
CPPFLAGS += $(addprefix -D,$(OPTIONS))

# Include the dependencies
ifneq ($(MAKECMDGOALS),clean)
sinclude $(addprefix $(BUILDDIR)/,$(call CONVERTEXT, d, $(SOURCES)))
endif

### Main Rule ###
.DEFAULT_GOAL := $(BUILDDIR)/$(TARGET)
$(BUILDDIR)/$(TARGET) : \
		$(addprefix $(BUILDDIR)/,$(call CONVERTEXT, o, $(SOURCES)))
	$(call OUTPUTINFO,LINK,$@)
	$(call OUTPUTINFO,CONFIG,$(CONFIG))
	@mkdir -p $(@D)
	$(Q)$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

### Utility Rules ###
.PHONY : clean
clean :
	-rm -rf buildresults/$(TARGET)

include buildsystem/autodep.mk
include buildsystem/rules.mk

endif # ifdef $(MKTARGETS)
