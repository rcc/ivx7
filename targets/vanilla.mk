# Output Name
TARGET := vanilla

# Sources
SOURCES :=	src/vanilla.c \
		lib/logging.c \
		lib/getline.c \
		lib/cmds.c \
		lib/cmds_script.c \
		lib/cmds_shell.c \

# Libraries
LIBRARIES := readline

# Options
OPTIONS := SCM_HASH='"$(shell git rev-parse --short=16 HEAD || \
		echo 0000000000000000)"'

# Configurations (the first one is the default)
CONFIGS := debug release

# Configuration Specific Options
RELEASE_OPTIONS := CONFIG_RELEASE MAX_LOGLEVEL=2

DEBUG_OPTIONS := CONFIG_DEBUG MAX_LOGLEVEL=5 DEFAULT_LOGLEVEL=4
