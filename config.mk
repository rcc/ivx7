# Output Name
TARGET := hello

# Sources
SOURCES := src/hello.c

# Libraries
LIBRARIES := m

# Options
OPTIONS := SCM_HASH='"$(shell git rev-parse --short=16 HEAD)"'

# Configurations (the first one is the default)
CONFIGS := debug release

# Configuration Specific Options
RELEASE_OPTIONS := CONFIG_RELEASE

DEBUG_OPTIONS := CONFIG_DEBUG DEBUG_PRINTS
