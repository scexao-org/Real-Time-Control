#
#	Makefile				Boston Micromachines Corp. library makefile
#
#	Mar 15,     2015        CIW/SJT original entry
#
#	© Copyright 2015, Boston Micromachines, Inc. All rights reserved.
#
###
#
BMC_LIBDIR	:= ../../lib
BMC_TMPDIR	:= ../../tmp
BMC_INCDIR	:= ../inc

CFLAGS	+= -I$(BMC_INCDIR)

$(BMC_TMPDIR)/%.o: %.c
	gcc -c $(CFLAGS) -o $@ $<
#
###
#
all:	$(BMC_LIBDIR)/libbmcmd.a

$(BMC_LIBDIR)/libbmcmd.a: $(BMC_TMPDIR)/bmc_mdlib.o
	ar -cr $@ $(BMC_TMPDIR)/bmc_mdlib.o
	ranlib $@

$(BMC_TMPDIR)/bmc_mdlib.o: bmc_mdlib.c \
				$(BMC_INCDIR)/bmc_mdlib.h $(BMC_INCDIR)/bmc_mdrv.h \

