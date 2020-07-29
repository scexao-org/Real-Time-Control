# Makefile - make rules for building examples for DPIO2 RHL console program

# Define the target platform
TARGET_PLATFORM = x86-linux-5.4.x
CPU = I386
ARCH = i386

# Define project home
PRJ_HOME = $(PWD)/..
#/home/scexao/DMcontrol

LIBDIR = $(PRJ_HOME)/dpio2-drv-src-x86-linux/lib/x86-linux-5.4.x

# If cross compiling
LD      =$(CROSS_COMPILE)ld
CC      =$(CROSS_COMPILE)gcc
CPP     =$(CROSS_COMPILE)g++

# Define example files
EXAMPLE_SOURCE = rhcli.c dpio2funcs.cpp thread.c util.c
EXAMPLE_OBJS = rhcli.o dpio2funcs.o thread.o util.o
EXAMPLE_EXE = rhcli

# self-refresh prog files
trigger_src = dm_trigger.c dpio2funcs.cpp thread.c util.c
trigger_obj = dm_trigger.o dpio2funcs.o thread.o util.o
trigger_exe = dm_trigger

# Define name of DPIO2 driver.
DRIVER = $(LIBDIR)/dpio2-drv.o

# Define the various flags that will be used.
CFLAGS += 	$(CC_OPTIM) \
		$(CC_WARNINGS) \
		$(CC_COMPILER) \
		$(CC_DEFINES) \
		$(CC_INCLUDE)

CPPFLAGS += 	$(CC_OPTIM) \
		$(CC_WARNINGS) \
		$(CPP_COMPILER) \
		$(CC_DEFINES) \
		$(CC_INCLUDE)

# Linker (ld) flags
LDFLAGS += -pthread -lrt

CC_OPTIM =	-fno-builtin \
		-fstrength-reduce \
		-fPIE

CC_WARNINGS = 	-Wall \
		-Wno-long-long

CC_INCLUDE =    -I$(PRJ_HOME)/dpio2-drv-src-x86-linux/include

CC_COMPILER =	-pthread

CPP_COMPILER =	-pthread

EXTRA_DEFINE +=	-D VMOSA
EXTRA_DEFINE +=	-D VM_LINUX
#EXTRA_DEFINE += -D MDL_DEBUG
#EXTRA_DEFINE += -D RSR_DEBUG


CC_DEFINES = 	-D CPU=$(CPU) \
		$(DEFINE_CC) \
		$(EXTRA_DEFINE)

build : $(EXAMPLE_OBJS) $(EXAMPLE_EXE) mkdata data1024.txt data4096.txt

build2: $(trigger_obj) $(trigger_exe)

$(EXAMPLE_EXE): $(EXAMPLE_OBJS) mapdata.h shared.h
	g++  $(LDFLAGS) -o  $(EXAMPLE_EXE) $(DRIVER) $(EXAMPLE_OBJS)

$(trigger_exe):	$(trigger_objs) mapdata.h shared.h
	g++  $(LDFLAGS) -o  $(trigger_exe) $(DRIVER) $(trigger_obj)

%.o : ./%.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o : ./%.cpp
	$(CPP) $(CPPFLAGS) -o $@ -c $<

mkdata: mkdata.c
	cc -o mkdata mkdata.c

data1024.txt: mkdata
	rm -f data1024.txt
	rm -f data4096.txt
	./mkdata

clean :
	rm -f	$(EXAMPLE_OBJS) \
		$(EXAMPLE_EXE) \
		$(trigger_obj) \
		$(trigger_exe) \
		$(DRIVER)

	rm -f mkdata
	rm -f data1024.txt
	rm -f data4096.txt

