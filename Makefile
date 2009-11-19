# Output Name
TARGET := hello

# Sources (space separated)
SOURCES := src/hello.c

# Libraries (space separated)
LIBRARIES := m

# Options (space separated)
OPTIONS :=

# Verbose Option
ifeq ($(VERBOSE),1)
	Q :=
else
	Q := @
endif

# Configuration
ifeq ($(CONFIG),)
	CONFIG := RELEASE
endif

# Set default flags
CPPFLAGS := -Iinclude
CFLAGS := -Wall -O2
ASFLAGS :=
LDFLAGS :=
LDLIBS := $(addprefix -l,$(LIBRARIES))

# Custom Functions
CONVERTSRCEXT = $(patsubst %.S,%.$1,$(patsubst %.c,%.$1,$2))
TOUPPER = $(shell echo $1 | \
	  sed 'y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/')
USCORESUB = $(shell echo $1 | sed 'y/ -./___/')

# Machine Name and Tool Versions
MACHINE := $(call USCORESUB,$(shell uname -sm))
CCNAME := $(call USCORESUB,$(notdir $(realpath $(shell which $(CC)))))

# Make CONFIG all uppercase
CONFIGU := $(call TOUPPER,$(CONFIG))

# Build Directory
BUILDDIR := buildresults/$(MACHINE)/$(CCNAME)/$(CONFIGU)

# Add in the config define
CPPFLAGS += -DCONFIG_$(CONFIGU)

# Add in the options
CPPFLAGS += $(addprefix -D,$(OPTIONS))

# Include the dependencies
ifneq ($(MAKECMDGOALS),clean)
sinclude $(addprefix $(BUILDDIR)/,$(call CONVERTSRCEXT,d,$(SOURCES)))
endif

ifneq ($(MAKECMDGOALS),clean)
# only want to print this out on final run, so put it after the sinclude of the
# deps, and only when its not a clean
$(info [CONFIG]    $(CONFIGU))
endif

### Main Rule ###
.DEFAULT_GOAL := $(BUILDDIR)/$(TARGET)
$(BUILDDIR)/$(TARGET) : \
		$(addprefix $(BUILDDIR)/,$(call CONVERTSRCEXT,o,$(SOURCES)))
	@echo "[LINK]      $@"
	@mkdir -p $(@D)
	$(Q)$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

################################################################################
### Auto Dependency Rules ###
$(BUILDDIR)/%.d : %.c
	@mkdir -p $(@D)
	@rm -f $@ && \
	$(CC) -MM $(CPPFLAGS) $< | \
	sed 's,\($(*F)\)\.o[ ]*:[ ]*,$(@D)/\1.o $@ : ,g' > $@

$(BUILDDIR)/%.d : %.S
	@mkdir -p $(@D)
	@rm -f $@ && \
	$(CC) -MM $(CPPFLAGS) $< | \
	sed 's,\($(*F)\)\.o[ ]*:[ ]*,$(@D)/\1.o $@ : ,g' > $@
################################################################################

################################################################################
### Implicit Rules ###
$(BUILDDIR)/%.o : %.c
	@mkdir -p $(@D)
	@echo "[CC]        $<"
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(BUILDDIR)/%.o : %.S
	@mkdir -p $(@D)
	@echo "[AS]        $<"
	$(Q)$(CC) $(CPPFLAGS) -c -o $@ $<
################################################################################

### Utility Rules ###
.PHONY : clean
clean :
	-rm -rf buildresults
