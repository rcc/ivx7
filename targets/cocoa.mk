# Output Name
TARGET := cocoa

# Sources
SOURCES :=	src/cocoa.m \
		lib/cmds.c \
		lib/getline.c \
		lib/cmds_script.c \

# Libraries
LIBRARIES :=

# Frameworks
FRAMEWORKS := Foundation

# Options
OPTIONS := SCM_HASH='"$(shell git rev-parse --short=16 HEAD || \
		echo 0000000000000000)"'

# Configurations (the first one is the default)
CONFIGS := debug release

# Configuration Specific Options
RELEASE_OPTIONS := CONFIG_RELEASE

DEBUG_OPTIONS := CONFIG_DEBUG DEBUG_PRINTS
