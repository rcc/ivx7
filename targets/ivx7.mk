CONFIGS := debug release

# Sources
SOURCES :=	src/main.c \
		src/prepost.c \
		src/ivx7_app.c \
		lib/logging.c \
		lib/getline.c \
		lib/dict.c \
		lib/cmds.c \
		lib/cmds_script.c \
		lib/cmds_shell.c \
		lib/serial/serial.c \
		lib/serial/device_chooser.c \
		lib/hexdump.c \
		lib/timelib.c \
		src/vx7if/vx7if.c \
		src/vx7if/vxcharset.c \
		src/cmds/version.c \
		src/cmds/device.c \
		src/cmds/clone.c \
		src/cmds/io.c \
		src/cmds/info.c \
		src/cmds/raw.c \
		src/cmds/memory.c \

# Libraries
LIBRARIES := readline

# Frameworks
FRAMEWORKS := CoreFoundation IOKit

# Version
include buildsystem/git.mk
OPTIONS += SCMVERSION='"$(SCMVERSION)"'
OPTIONS += SCMBRANCH='"$(SCMBRANCH)"'
CPPFLAGS += -DBUILD_DATE='"$(shell date)"'

# Release Config
ifeq ($(CONFIG),release)
# Options
OPTIONS += MAX_LOGLEVEL=4 DEFAULT_LOGLEVEL=3
# Flags
CFLAGS += -O2
endif

# Debug Config
ifeq ($(CONFIG),debug)
# Options
OPTIONS += MAX_LOGLEVEL=5 DEFAULT_LOGLEVEL=4
# Flags
CFLAGS += -O0 -g
endif

INSTALL_SCRIPT = targets/install
