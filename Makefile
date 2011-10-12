include buildsystem/func.mk

# Host OS
export HOSTOS := $(call TOUPPER,$(call USCORESUB,$(shell uname -s)))

# Verbose Option
ifeq ($(VERBOSE),1)
export Q :=
export VERBOSE := 1
else
export Q := @
export VERBOSE := 0
endif

# Machine Name and Tool Versions
export MACHINE := $(call USCORESUB,$(shell uname -sm))
export CCNAME := $(call USCORESUB,$(notdir $(realpath $(shell which $(CC)))))

# Build Directory
BUILDDIR_ROOT := buildresults
export BUILDDIR := $(BUILDDIR_ROOT)/$(MACHINE)/$(CCNAME)

# Target Directory
TARGETDIR := targets

# Build Target List
TARGETS := $(basename $(notdir $(wildcard $(TARGETDIR)/*.mk)))

.DEFAULT_GOAL := help
.PHONY : help
help :
	@echo "usage: make <target[.config]>"
	@echo "       make all"
	@echo "       make clean"
	@echo "targets:"
	$(call PRINTLIST,$(TARGETS), * )

all : $(TARGETS)

.PHONY : $(TARGETS)
$(TARGETS) :
	$(Q)$(MAKE) -f buildsystem/target.mk \
		TARGETMK="$(TARGETDIR)/$@.mk"

$(addsuffix .%,$(TARGETS)) :
	$(Q)$(MAKE) -f buildsystem/target.mk \
		CONFIG=$(call EXTRACT_CONFIG,$@) \
		TARGETMK="$(TARGETDIR)/$(call EXTRACT_TARGET,$@).mk"

### Utility Rules ###
.PHONY : clean
clean :
	$(Q)-rm -rf $(BUILDDIR_ROOT)
	$(call OUTPUTINFO,CLEAN,$(BUILDDIR_ROOT))
