SCMVERSION := $(shell git describe --tags --dirty=+ 2>/dev/null || \
		git rev-parse --short=16 HEAD 2>/dev/null || \
		echo "UNKNOWN")
SCMBRANCH := $(shell (git branch 2> /dev/null || echo '* UNKNOWN') | \
		awk '/^\* / {print $$2;}')
