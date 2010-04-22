include buildsystem/func.mk

# Set default flags
CPPFLAGS := -Iinclude
CFLAGS := -Wall -O2
ASFLAGS :=
LDFLAGS :=

include config.mk # include after default flags so config.mk can append to them

# Add all the libraries defined in config.mk to LDLIBS
LDLIBS := $(addprefix -l,$(LIBRARIES))

# Add all the frameworks defined in config.mk to LDFLAGS
#   This is only for objective-C in OSX, but it doesn't hurt us here assuming
#   no one defines FRAMEWORKS
LDFLAGS += $(addprefix -framework ,$(FRAMEWORKS))

# Verbose Option
ifeq ($(VERBOSE),1)
	Q :=
else
	Q := @
endif

# Configuration
ifeq ($(words $(CONFIGS)),0)
$(error Must specify at least one config in config.mk)
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
BUILDDIR := buildresults/$(MACHINE)/$(CCNAME)/$(CONFIG)

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
	-rm -rf buildresults

include buildsystem/autodep.mk
include buildsystem/rules.mk
