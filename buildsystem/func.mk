### Custom Functions ###

# convert source file extensions to another extension
CONVERTEXT = $(addsuffix .$(strip $1), $(basename $2))

TOUPPER = $(shell echo $1 | \
	  sed 'y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/')

RMTRAILINGSLASH = $(patsubst %/,%,$1)

USCORESUB = $(shell echo $1 | sed 'y/ -./___/')

OUTPUTINFO = @echo "[$1],$2" | \
	     awk 'BEGIN {FS = ",";} {printf("%-15s %s\n",$$1,$$2);}'
