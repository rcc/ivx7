# Output Name
TARGET := cocoa

# Sources
SOURCES :=	src/cocoa.m \
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
OPTIONS := VERSION='"0.1"'
OPTIONS += LOG_WITH_NSLOG

# Configurations (the first one is the default)
CONFIGS := debug release

# Configuration Specific Options
RELEASE_OPTIONS := CONFIG_RELEASE MAX_LOGLEVEL=2

DEBUG_OPTIONS := CONFIG_DEBUG MAX_LOGLEVEL=5 DEFAULT_LOGLEVEL=4
