######################################################################################################################################
# Copyright @Vbextreme 2017
# GCC Makefile
# is public domain), believed to be neutral to any flavor of "make"
# (GNU make, BSD make, SysV make)
######################################################################################################################################

all:
	@make -f build.mk PRJ_TYPE=shared
	#@make -f build.mk PRJ_TYPE=static

dbg:
	@make -f build.mk PRJ_TYPE=app USER_DEFINES="-DDEBUG_ENABLE=1 -DASSERT_ENABLE=1"

tst:
	@make -f build.mk PRJ_TYPE=app

clean:
	@make -f build.mk PRJ_TYPE=app clean
	@make -f build.mk PRJ_TYPE=shared clean
	
install:
	@make -f build.mk PRJ_TYPE=shared install
	
uninstall:
	@make -f build.mk PRJ_TYPE=shared uninstall
	
sources:
	@make -f build.mk PRJ_TYPE=shared sources

info:
	@make -f build.mk PRJ_TYPE=shared info

size:
	@make -f build.mk PRJ_TYPE=shared size

prof:
	@make -f build.mk PRJ_TYPE=app USER_DEFINES="-D_PROF" CEXTRA=-pg LDEXTRA=-pg
	@make -f build.mk PRJ_TYPE=app prof

prof.view:
	@make -f build.mk PRJ_TYPE=app prof.view

valgrind:
	@make -f build.mk PRJ_TYPE=app valgrind 
	
valgrind.view:
	@make -f build.mk PRJ_TYPE=app valgrind.view

.PHONY : all dbg tst clean install uninstall sources info size prof prof.view valgrind valgrind.view
