# Output Name
TARGET := vanilla

# Sources
SOURCES :=	src/vanilla.c \
		lib/cmds.c \

# Libraries
LIBRARIES :=

# Options
OPTIONS := SCM_HASH='"$(shell git rev-parse --short=16 HEAD || \
		echo 0000000000000000)"'

# Configurations (the first one is the default)
CONFIGS := debug release

# Configuration Specific Options
RELEASE_OPTIONS := CONFIG_RELEASE

DEBUG_OPTIONS := CONFIG_DEBUG DEBUG_PRINTS
