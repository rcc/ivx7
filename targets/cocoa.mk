# Output Name
TARGET := cocoa

# Sources
SOURCES :=	src/cocoa.m \
		src/AppMgr.m \
		lib/logging.c \
		lib/getline.c \
		lib/cmds.c \
		lib/cmds_script.c \
		lib/cmds_shell.c \

# Libraries
LIBRARIES := readline

# Frameworks
FRAMEWORKS := Foundation

# Options
VERSION := $(shell git describe --tags HEAD || echo "Unknown")
OPTIONS += VERSION='"$(VERSION)"'
OPTIONS += LOG_WITH_NSLOG

# Configurations (the first one is the default)
CONFIGS := debug release

# Configuration Specific Options
RELEASE_OPTIONS := CONFIG_RELEASE
RELEASE_OPTIONS += MAX_LOGLEVEL=3 DEFAULT_LOGLEVEL=2

DEBUG_OPTIONS := CONFIG_DEBUG
DEBUG_OPTIONS += MAX_LOGLEVEL=5 DEFAULT_LOGLEVEL=4
