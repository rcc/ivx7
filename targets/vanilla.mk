# Output Name
TARGET := vanilla

# Sources
SOURCES :=	src/main.c \
		lib/logging.c \
		lib/getline.c \
		lib/dict.c \
		lib/cmds.c \
		lib/cmds_script.c \
		lib/cmds_shell.c \
		src/cmds/version.c \

# Libraries
LIBRARIES := readline

# Version
SCMVERSION := $(shell git describe --tags --dirty=+ HEAD 2>/dev/null || \
		git rev-parse --short=16 HEAD 2>/dev/null || \
	    	echo "UNKNOWN")
OPTIONS += SCMVERSION='"$(SCMVERSION)"'
CPPFLAGS += -DBUILD_DATE='"$(shell date)"'

# Options

# Configurations (the first one is the default)
CONFIGS := debug release

# Configuration Specific Options
RELEASE_OPTIONS := CONFIG_RELEASE
RELEASE_OPTIONS += MAX_LOGLEVEL=3 DEFAULT_LOGLEVEL=2

DEBUG_OPTIONS := CONFIG_DEBUG
DEBUG_OPTIONS += MAX_LOGLEVEL=5 DEFAULT_LOGLEVEL=4

# Configuration Specific Flags
RELEASE_CFLAGS := -O2
RELEASE_CXXFLAGS := $(RELEASE_CFLAGS)

DEBUG_CFLAGS := -O0 -g
DEBUG_CXXFLAGS := $(DEBUG_CFLAGS)
