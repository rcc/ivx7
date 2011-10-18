include buildsystem/func.mk

# Set default flags
CPPFLAGS := -Iinclude -Isrc
CFLAGS := -Wall -Werror
CXXFLAGS = $(CFLAGS)
ASFLAGS :=
LDFLAGS :=
OPTIONS :=
ANALYZE_FLAGS :=	-analyzer-viz-egraph-graphviz \
			-analyzer-opt-analyze-headers \
			-analyzer-inline-call \
			-analyzer-output=html

ifeq ($(TARGET),)
$(error no TARGET defined)
endif
ifeq ($(CONFIG),)
$(error no CONFIG defined)
endif
ifeq ($(TARGETMK),)
$(error no TARGETMK defined)
endif
include $(TARGETMK)

# Add all the libraries defined in config.mk to LDLIBS
LDLIBS := $(addprefix -l,$(LIBRARIES))

# Add all the frameworks defined in config.mk to LDFLAGS (This is only for OSX)
ifeq ($(HOSTOS),DARWIN)
LDFLAGS += $(addprefix -framework ,$(FRAMEWORKS))
endif

# Build Directory
BUILDDIR := $(BUILDDIR)/$(TARGET)/$(CONFIG)

# Add in the target name and host OS
OPTIONS += __TARGET__='"$(TARGET)"' __CONFIG__='"$(CONFIG)"'
OPTIONS += __TARGET_$(call TOUPPER,$(TARGET))__
OPTIONS += __CONFIG_$(call TOUPPER,$(CONFIG))__
OPTIONS += __HOST_$(HOSTOS)__

# Add in the options
CPPFLAGS += $(addprefix -D,$(OPTIONS))

# Include the dependencies
sinclude $(addprefix $(BUILDDIR)/,$(call CONVERTEXT, d, $(SOURCES)))

### Main Rule ###
.DEFAULT_GOAL := $(BUILDDIR)/$(TARGET)
$(BUILDDIR)/$(TARGET) : \
		$(addprefix $(BUILDDIR)/,$(call CONVERTEXT, o, $(SOURCES)))
	$(call OUTPUTINFO,LINK,$@)
	@mkdir -p $(@D)
	$(Q)$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@
ifeq ($(ANALYZE),1)
	$(call OUTPUTINFO,ANALYZE,generating report)
	$(Q)buildsystem/gen_analyzer_report $(BUILDDIR)
endif

### Utility Rules ###
ifneq ($(INSTALL_SCRIPT),)
.PHONY : install
install : $(BUILDDIR)/$(TARGET)
	$(call OUTPUTINFO,INSTALL,$<)
	$(Q)$(INSTALL_SCRIPT) $<
endif

include buildsystem/autodep.mk
include buildsystem/rules.mk
