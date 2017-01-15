######################################################################################################################################
# Copyright @Vbextreme 2017
# GCC Makefile
# is public domain), believed to be neutral to any flavor of "make"
# (GNU make, BSD make, SysV make)
######################################################################################################################################

DIR_MAKEFILE = $(CURDIR)

####################
# Extra Makefile  #
####################

ifeq ($(PRJ_TYPE),app)
	PRJ_SUFFIX = 
	include ./configAPP.mk
else ifeq ($(PRJ_TYPE),dbg)
	PRJ_SUFFIX = 
	include ./configAPP.mk
else ifeq ($(PRJ_TYPE),shared)
	PRJ_SUFFIX = .so
	include ./configSHARED.mk
else ifeq ($(PRJ_TYPE),static)
	PRJ_SUFFIX = .a
	include ./configSTATIC.mk
endif

include ./sys.mk
include ./function.mk

#########
# Make  #
#########

# file #
VPATH = $(PRJ_SRC)
SOURCES = $(PRJ_SOURCES)
OBJECTS = $(PRJ_OBJECTS)
TARGET = $(PRJ_BUILD)/$(PRJ_TARGET)

# var compiler #
CDEFS = $(PRJ_DEFINES) $(USER_DEFINES)
CINCS = -I$(PRJ_HEADERS)
CFLAGS = $(CEXTRA) -mcpu=$(PRJ_COMPILER_CPU) -mtune=$(PRJ_COMPILER_TUNE) $(PRJ_OPTIONS) $(CDEFS) $(CINCS)
LDFLAGS = $(LDEXTRA) -mcpu=$(PRJ_COMPILER_CPU) -mtune=$(PRJ_COMPILER_TUNE) $(PRJ_LNK_OPTIONS)
LDLIBS = $(PRJ_LIBS_SHARED) $(PRJ_LIBS_STATIC) 
HFILE = 
ARFLAGS = $(PRJ_LNK_OPTIONS)

# eye #
EYE_FCOUNT_O = $(words $(OBJECTS))
EYE_INDEX = 

# mode #

all: begin gccversion mkd builds size end

begin:
	$(EMPTYLINE)
	$(call COLWRITE,$(BCOL_GRAY),$(FCOL_YELLOW),$(PRJ_TYPE))
	$(call COLMESSAGE,$(BCOL_GRAY),$(FCOL_YELLOW), "\ Begin\ Build:")
	@./counter 'z' $(PRJ_TARGET)

gccversion : 
	@$(CC) --version
	
builds: $(TARGET)$(PRJ_SUFFIX)

$(TARGET): $(OBJECTS)
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE), "\ Linker:\ $@")
	$(LD) $(LDFLAGS) -o $(TARGET)$(PRJ_SUFFIX) $(OBJECTS) $(LDLIBS)

$(TARGET).so: $(OBJECTS)
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE), "\ Linker:\ $@")
	$(LD) $(LDFLAGS) -o $(TARGET)$(PRJ_SUFFIX) $(OBJECTS) $(LDLIBS)

$(TARGET).a: $(OBJECTS)
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE), "\ Linker:\ $@")
	$(AR) $(ARFLAGS) $(TARGET)$(PRJ_SUFFIX) $(OBJECTS)
	$(RANLIB) $(TARGET)$(PRJ_SUFFIX)

$(PRJ_BUILD)/%.o: %.c $(HFILE)
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),"")
	@./counter 'p' $(PRJ_TARGET) $(EYE_FCOUNT_O)
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE), "\ Build\ Object:\ $<")
	$(CC) $(CFLAGS) -c $< -o $@ $(CDEFS) 

mkd:
	@$(MKDIR) -p $(PRJ_BUILD)
	
size:
	$(EMPTYLINE)
	$(call COLMESSAGE,$(BCOL_GRAY),$(FCOL_YELLOW),"Size") 
	@$(SIZE) $(TARGET)$(PRJ_SUFFIX)

end:
	$(EMPTYLINE)
	$(call COLMESSAGE,$(BCOL_GRAY),$(FCOL_YELLOW), "\ End\ Build")
	$(EMPTYLINE)
	@./counter 'r' $(PRJ_TARGET)
	
clean: begin_clean clean_list end_clean

begin_clean:
	$(EMPTYLINE)
	$(call COLWRITE,$(BCOL_GRAY),$(FCOL_YELLOW),$(PRJ_TYPE))
	$(call COLMESSAGE,$(BCOL_GRAY),$(FCOL_YELLOW), "\ Begin\ Clean:")
	@./counter 'z' $(PRJ_TARGET)
	
clean_list :
	$(RM) $(PRJ_BUILD)/*
	@./counter r $(PRJ_TARGET)

end_clean:
	$(EMPTYLINE)
	$(call COLMESSAGE,$(BCOL_GRAY),$(FCOL_YELLOW), "End\ Clean")
	$(EMPTYLINE)
	@./counter 'r' $(PRJ_TARGET)

color:
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_GRAY),"GRAY")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_RED),"RED")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_GREEN),"GREEN")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_YELLOW),"YELLOW")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE),"BLUE")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_MAGENTA),"MAGENTA")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_CYAN),"CYAN")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_LGRAY),"LGRAY")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_LRED),"LRED")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_LGREEN),"LGREEN")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_LYELLOW),"LYELLOW")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_LBLUE),"LBLUE")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_LMAGENTA),"LMAGENTA")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_LCYAN),"LCYAN")	
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_WHITE),"WHITE")

info:
	$(EMPTYLINE)
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE),"-------------")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE),"-\ Directory\ -")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE),"-------------")
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),"-\ ProgectSrc:\ ")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_WHITE),$(VPATH))
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),"-\ build:\ ")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_WHITE),$(PRJ_BUILD))
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),"-\ include:\ ")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_WHITE),$(CINCS))
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),"-\ lib:\ ")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_WHITE),$(PRJ_LIBRARY))
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE),"-----------")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE),"-\ Project\ -")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE),"-----------")
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),"-\ target:\ ")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_WHITE),$(PRJ_TARGET)$(PRJ_SUFFIX))
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),"-\ arch:\ ")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_WHITE),$(PRJ_COMPILER_CPU))
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),"-\ type:\ ")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_WHITE),$(PRJ_COMPILER_TUNE))
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),"-\ ld:\ ")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_WHITE),$(LD))
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),"-\ options\ gcc:\ ")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_WHITE),$(CFLAGS))
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),"-\ options\ ld:\ ")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_WHITE),$(LDFLAGS))
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),"-\ defines:\ ")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_WHITE),$(CDEFS))
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),"-\ library\ shared:\ ")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_WHITE),$(PRJ_LIBS_SHARED))
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),"-\ library\ static:\ ")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_WHITE),$(PRJ_LIBS_STATIC))
	$(EMPTYLINE)

sources:
	$(EMPTYLINE)
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE),"-----------")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE),"-\ Project\ -")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE),"-----------")
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),"-\ sources:\ ")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_WHITE),$(SOURCES))
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),"-\ object:\ ")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_WHITE),$(OBJECTS))
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),"-\ headers:\ ")
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_WHITE),$(PRJ_HEADERS_NAME))
	$(EMPTYLINE)

install: stinstall $(PRJ_TARGET)$(PRJ_SUFFIX).install eninstall

stinstall:
	$(EMPTYLINE)
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),$(PRJ_TYPE))
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE),"\ install...")

eninstall:
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE),"Complete")
	$(EMPTYLINE)

$(PRJ_TARGET).install:
	@$(CP) -f $(TARGET)$(PRJ_SUFFIX) $(PRJ_INSTALL_BINARY)/$(PRJ_TARGET)$(PRJ_SUFFIX)
	@$(CHMOD) u=rwx,go=rx $(PRJ_INSTALL_BINARY)/$(PRJ_TARGET)$(PRJ_SUFFIX)
	
$(PRJ_TARGET).so.install:
	$(MKDIR) -p $(PRJ_INSTALL_HEADER_DIR)
	@$(CHMOD) u=rwx,go=rx $(PRJ_INSTALL_HEADER_DIR)
	$(CP) $(PRJ_SRC_HEADERS_FILE) $(PRJ_INSTALL_HEADER_DIR)
	@$(CHMOD) u=rw,go=r $(PRJ_INSTALL_HEADER_DIR)/*
	$(CP) $(PRJ_BUILD)/$(PRJ_TARGET)$(PRJ_SUFFIX) $(PRJ_INSTALL_LIBRARY)
	@$(CHMOD) u=rw,go=r $(PRJ_INSTALL_LIBRARY)/$(PRJ_TARGET)$(PRJ_SUFFIX)
	
$(PRJ_TARGET).a.install:
	$(MKDIR) -p $(PRJ_INSTALL_HEADER_DIR)
	@$(CHMOD) u=rwx,go=rx $(PRJ_INSTALL_HEADER_DIR)
	$(CP) $(PRJ_SRC_HEADERS_FILE) $(PRJ_INSTALL_HEADER_DIR)
	@$(CHMOD) u=rw,go=r $(PRJ_INSTALL_HEADER_DIR)/*
	$(CP) $(PRJ_BUILD)/$(PRJ_TARGET)$(PRJ_SUFFIX) $(PRJ_INSTALL_LIBRARY)
	@$(CHMOD) u=rw,go=r $(PRJ_INSTALL_LIBRARY)/$(PRJ_TARGET)$(PRJ_SUFFIX)
	
uninstall: stuninstall $(PRJ_TARGET)$(PRJ_SUFFIX).uninstall enuninstall

stuninstall:
	$(EMPTYLINE)
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),$(PRJ_TYPE))
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE),"\ uninstall...")

enuninstall:
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE),"complete")
	$(EMPTYLINE)

$(PRJ_TARGET).uninstall:
	@$(CP) $(PRJ_INSTALL_BINARY)/$(PRJ_TARGET)$(PRJ_SUFFIX)
	@$(CHMOD) u=rwx,go=rx $(PRJ_INSTALL_BINARY)/$(PRJ_TARGET)$(PRJ_SUFFIX)
	
$(PRJ_TARGET).so.uninstall:
	@$(RM) $(PRJ_INSTALL_LIBRARY)/$(PRJ_TARGET)$(PRJ_SUFFIX)
	@$(RM) $(addprefix $(PRJ_INSTALL_HEADER_DIR)/, $(PRJ_HEADERS_NAME))

$(PRJ_TARGET).a.uninstall:
	@$(RM) $(PRJ_INSTALL_LIBRARY)/$(PRJ_TARGET)$(PRJ_SUFFIX)
	@$(RM) $(addprefix $(PRJ_INSTALL_HEADER_DIR)/, $(PRJ_HEADERS_NAME))

prof:
	$(EMPTYLINE)
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),$(PRJ_TYPE))
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE),"\ start\ software\ press\ ctrl\ C\ to\ break")
	$(TARGET)$(PRJ_SUFFIX)
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE),"profiling...")
	@$(MV) $(DIR_MAKEFILE)/gmon.out $(PRJ_BUILD)/gmon.out
	@$(GP) $(TARGET)$(PRJ_SUFFIX) $(PRJ_BUILD)/gmon.out > $(PRJ_BUILD)/profiling
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE),"Display")
	@$(CAT) $(PRJ_BUILD)/profiling

prof.view:
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE),"Display")
	@($CAT) $(PRJ_BUILD)/profiling
 
valgrind:
	$(EMPTYLINE)
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),$(PRJ_TYPE))
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE),"valgrind...")
	$(CD) $(PRJ_BUILD) && $(VG) --tool=callgrind ./$(PRJ_TARGET)$(PRJ_SUFFIX)

valgrind.view:
	$(EMPTYLINE)
	$(call COLWRITE,$(BCOL_BLACK),$(FCOL_BLUE),$(PRJ_TYPE))
	$(call COLMESSAGE,$(BCOL_BLACK),$(FCOL_BLUE),"Display")
	$(KC) $(PRJ_BUILD)/callgrind.out.* 
 
############################
# Listing of phony targets #
############################
.PHONY : all gccversion size builds info color sources clean install uninstall
