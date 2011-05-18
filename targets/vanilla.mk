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

# Libraries
LIBRARIES := readline

# Options
VERSION := $(shell git describe --tags HEAD 2>/dev/null || echo "Unknown")
OPTIONS += VERSION='"$(VERSION)"'

# Configurations (the first one is the default)
CONFIGS := debug release

# Configuration Specific Options
RELEASE_OPTIONS := CONFIG_RELEASE
RELEASE_OPTIONS += MAX_LOGLEVEL=3 DEFAULT_LOGLEVEL=2

DEBUG_OPTIONS := CONFIG_DEBUG
DEBUG_OPTIONS += MAX_LOGLEVEL=5 DEFAULT_LOGLEVEL=4
