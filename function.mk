######################################################################################################################################
# Copyright @Vbextreme 2017
# GCC Makefile
# is public domain), believed to be neutral to any flavor of "make"
# (GNU make, BSD make, SysV make)
######################################################################################################################################

###########
# MESSAGE #
###########
BCOL_BLACK = 40
BCOL_GRAY = 100
BCOL_RED = 41
BCOL_GREEN = 42
BCOL_YELLOW = 43
BCOL_BLUE = 44
BCOL_MAGENTA = 45
BCOL_CYAN = 46
BCOL_LGRAY = 47
BCOL_LRED = 101
BCOL_LGREEN = 102
BCOL_LYELLOW = 103
BCOL_LBLUE = 104
BCOL_LMAGENTA = 105
BCOL_LCYAN = 106
BCOL_WHITE = 107

FCOL_BLACK = 30
FCOL_GRAY = 90
FCOL_RED = 31
FCOL_GREEN = 32
FCOL_YELLOW = 33
FCOL_BLUE = 34
FCOL_MAGENTA = 35
FCOL_CYAN = 36
FCOL_LGRAY = 37
FCOL_LRED = 91
FCOL_LGREEN = 92
FCOL_LYELLOW = 93
FCOL_LBLUE = 94
FCOL_LMAGENTA = 95
FCOL_LCYAN = 96
FCOL_WHITE = 97

COL_RESET = 

EMPTYLINE = @echo ""
COLRESET = @printf "\033[m"
COLMESSAGE = @printf "\033[$1m\033[$2m$3\033[m\n" 
COLWRITE = @printf "\033[$1m\033[$2m$3" 
