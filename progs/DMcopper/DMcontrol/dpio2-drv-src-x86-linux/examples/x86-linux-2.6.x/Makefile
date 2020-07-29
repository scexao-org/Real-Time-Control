# Makefile - make rules for building examples for DPIO2 device driver 
#            for x86-linux.
#
# modification history
# --------------------
# 01a,10sep2004,meh  Created.
#



# Define the target platform
TARGET_PLATFORM = x86-linux-2.6.x
CPU = I386
ARCH = i386
#CROSS_COMPILE =


# Define project home
PRJ_HOME = ../..
LIBDIR = $(PRJ_HOME)/lib/$(TARGET_PLATFORM)


# If cross compiling
LD      =$(CROSS_COMPILE)ld
CC      =$(CROSS_COMPILE)gcc
CPP     =$(CROSS_COMPILE)g++


# Define example files
EXAMPLE_SOURCE = x86LinuxExampleFromManual.c
EXAMPLE_OBJS = x86LinuxExampleFromManual.o
EXAMPLE_EXE = x86LinuxExampleFromManual


# Define name of DPIO2 driver.
DRIVER = $(LIBDIR)/dpio2-drv.o


# Define the various flags that will be used.
CFLAGS += 	$(CC_OPTIM) \
		$(CC_WARNINGS) \
		$(CC_COMPILER) \
		$(CC_DEFINES) \
		$(CC_INCLUDE)

C++FLAGS += 	$(CC_OPTIM) \
		$(CC_WARNINGS) \
		$(C++_COMPILER) \
		$(CC_DEFINES) \
		$(CC_INCLUDE)

LDFLAGS += -pthread

CC_OPTIM =	-fno-builtin \
		-fstrength-reduce

CC_WARNINGS = 	-Wall \
		-Wno-long-long

CC_INCLUDE =	-I.. \
		-I$(PRJ_HOME)/include \
		-I$(PRJ_HOME)/test/verification/include

CC_COMPILER =	-ansi \
		-fno-builtin \
		-pthread

C++_COMPILER =	-ansi \
		-fno-builtin \
		-pthread


EXTRA_DEFINE +=	-D VMOSA
EXTRA_DEFINE +=	-D VM_LINUX


CC_DEFINES = 	-D CPU=$(CPU) \
		$(DEFINE_CC) \
		$(EXTRA_DEFINE)



build : $(EXAMPLE_OBJS)
	make ensure_dir DIR=$(LIBDIR);	
	g++ $(LDFLAGS) -o  $(EXAMPLE_EXE) $(DRIVER) $(EXAMPLE_OBJS)
	cp $(EXAMPLE_EXE) $(LIBDIR)/$(EXAMPLE_EXE)
	cp $(EXAMPLE_SOURCE) $(PRJ_HOME)/doc/$(EXAMPLE_SOURCE)

%.o : ./%.c
	$(CC) $(CFLAGS) -o $@ -c $<


clean :
	rm 	$(EXAMPLE_OBJS) \
		$(EXAMPLE_EXE)


ensure_dir:
	@if test ! -d $(DIR) ; then \
	echo "Create" $(DIR);     \
	mkdir -p $(DIR); \
	fi