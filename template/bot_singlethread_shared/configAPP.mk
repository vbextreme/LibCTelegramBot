######################################################################################################################################
# Copyright @Vbextreme 2017
# GCC Makefile
# is public domain), believed to be neutral to any flavor of "make"
# (GNU make, BSD make, SysV make)
######################################################################################################################################

#####################
# Project Directory #
#####################

PRJ_TARGET = tgb
PRJ_SRC = $(DIR_MAKEFILE)/src
PRJ_BUILD = $(DIR_MAKEFILE)/build_app
PRJ_HEADERS = $(DIR_MAKEFILE)
PRJ_LIBRARY = 
PRJ_INSTALL_BINARY = /usr/bin

##############
# CPU CONFIG #
##############

PRJ_COMPILER_CPU ?= native
PRJ_COMPILER_TUNE ?= native

#################
### C OPTIONS ###
#################

PRJ_OPTIONS = -Wall
PRJ_OPTIONS += -Wextra
PRJ_OPTIONS += -g
#PRJ_OPTIONS += -g3
PRJ_OPTIONS += -pthread
#PRJ_OPTIONS += -funsigned-char
#PRJ_OPTIONS += -fno-common
#PRJ_OPTIONS += -fpack-struct
#PRJ_OPTIONS += -ffunction-sections
#PRJ_OPTIONS += -fdata-sections
#PRJ_OPTIONS += -funsigned-bitfields
#PRJ_OPTIONS += -nostdlib
#PRJ_OPTIONS += -nostartfiles
#PRJ_OPTIONS += -fshort-enums
#PRJ_OPTIONS += -Wstrict-prototypes
#PRJ_OPTIONS += -Wundef
#PRJ_OPRIONS += -std=c89
#PRJ_OPTIONS += -std=gnu89
#PRJ_OPTIONS += -std=c99
#PRJ_OPTIONS += -std=gnu99
#PRJ_OPTIONS += -std=c11
PRJ_OPTIONS += -std=gnu11
#PRJ_OPTIONS += -O3
#PRJ_OPTIONS += -O2
#PRJ_OPTIONS += -O1
#PRJ_OPTIONS += -Os

#pragma message
PRJ_OPTIONS += -fno-diagnostics-show-caret

######################
### LINKER OPTIONS ###
######################

PRJ_LNK_OPTIONS = -Wall
PRJ_LNK_OPTIONS += -Wextra
PRJ_LNK_OPTIONS += -pthread
#PRJ_LNK_OPTIONS += -nostdlib
PRJ_LNK_OPTIONS += -g
#PRJ_LNK_OPTIONS += -g3
#PRJ_LNK_OPTIONS += -O3
#PRJ_LNK_OPTIONS += -O2
#PRJ_LNK_OPTIONS += -O1
#PRJ_LNK_OPTIONS += -Os

###################
### DEFINITIONS ###
###################

### default ###
PRJ_DEFINES = -D_APP
#PRJ_DEFINES += -DDEBUG_ENABLE=1 
#PRJ_DEFINES += -DDEBUG_ENABLE=1 
#PRJ_DEFINES += -DASSERT_ENABLE=1
#PRJ_DEFINES += -DDISABLE_PRAGMA_MESSAGE

######################
### LIBRARY SHARED ###
######################

PRJ_LIBS_SHARED = -ltgb 
PRJ_LIBS_STATIC =

#############
### FILE  ###
#############

PRJ_SOURCES = $(wildcard $(PRJ_SRC)/*.c )
PRJ_OBJECTS = $(patsubst $(PRJ_SRC)/%.c, $(PRJ_BUILD)/%.o, $(PRJ_SOURCES))
PRJ_OBJECTS_NAME =  $(notdir $(PRJ_OBJECTS))
