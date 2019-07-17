#
#	Makefile				Boston Micromachines Corp. project makefile
#
#	Mar 15,     2015        CIW/SJT original entry
#
#	© Copyright 2015, Boston Micromachines, Inc. All rights reserved.
#
###
#
BMC_LIBDIR	:= ../lib
BMC_TMPDIR	:= ../tmp
BMC_INCDIR	:= ./inc
BMC_BINDIR	:= ../bin
#
###
#
#	Targets
#
build: $(BMC_TMPDIR) $(BMC_LIBDIR) $(BMC_BINDIR) $(BMC_BINDIR)/bmc_ltest

clean:
	rm -fR $(BMC_TMPDIR)
	rm -fR $(BMC_LIBDIR)
	rm -fR $(BMC_BINDIR)
	rm -f drv/*.o
	rm -f drv/*.mod.c
	rm -f drv/*.ko
	rm -f drv/.*.o
	rm -f drv/.*.cmd
	rm -f drv/modules.order
	rm -f drv/Module.symvers
	rm -fR drv/.tmp_versions
#
###
#
#	Build specifications
#

$(BMC_TMPDIR):
	mkdir $(BMC_TMPDIR)

$(BMC_LIBDIR):
	mkdir $(BMC_LIBDIR)

$(BMC_BINDIR):
	mkdir $(BMC_BINDIR)

$(BMC_BINDIR)/bmc_ltest:
	cd drv; make  -C /lib/modules/`uname -r`/build M=`pwd`
	cp -p drv/bmc_mdrv.ko $(BMC_BINDIR)
	cd lib; make
	cd tool; make

