# Output Name
TARGET := hello

# Sources
SOURCES := src/hello.c

# Libraries
LIBRARIES := m

# Options
OPTIONS :=

# Configurations (the first one is the default)
CONFIGS := release debug

# Configuration Specific Options
RELEASE_OPTIONS := CONFIG_RELEASE

DEBUG_OPTIONS := CONFIG_DEBUG DEBUG_PRINTS
