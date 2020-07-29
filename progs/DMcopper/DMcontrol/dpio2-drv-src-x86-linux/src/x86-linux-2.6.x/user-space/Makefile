# Makefile - make rules for building DPIO2 device driver for x86-Linux.
#
# modification history
# --------------------
#
# 01a,06jul2004,meh  Created based on makefile for midas-ppc440.
#



# Define the target platform
TARGET_PLATFORM = x86-linux-2.6.x
CPU = I386
ARCH = i386
#CROSS_COMPILE =


# Define project home
PRJ_HOME = ../../..
LIBDIR = $(PRJ_HOME)/lib/$(TARGET_PLATFORM)


# If cross compiling
LD      =$(CROSS_COMPILE)ld
CC      =$(CROSS_COMPILE)gcc
CPP     =$(CROSS_COMPILE)g++


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

LDFLAGS += -r

CC_OPTIM =	-fno-builtin \
		-fstrength-reduce

CC_WARNINGS = 	-Wall \
		-Wno-long-long

CC_INCLUDE =	-I.. \
		-I$(PRJ_HOME)/src \
		-I$(PRJ_HOME)/include \
		-I$(PRJ_HOME)/src/$(TARGET_PLATFORM)/include

#CC_COMPILER =	-ansi 
CC_COMPILER =	-fno-builtin \
		-pthread \
		-c

#C++_COMPILER =	-ansi 
C++_COMPILER =	-fno-builtin \
		-pthread \
		-c


EXTRA_DEFINE +=	-D VMOSA
EXTRA_DEFINE +=	-D VM_LINUX 


CC_DEFINES = 	-D CPU=$(CPU) \
		$(DEFINE_CC) \
		$(EXTRA_DEFINE)


# Define name of the result we seek.
DRIVER = 	$(LIBDIR)/dpio2-drv.o

# Object file containing the VxWorks I/O System Part of the driver.
DRIVER_LAYER =	$(LIBDIR)/dpio2-io.o

# Object file containing the driver core.
DRIVER_CORE = 	$(LIBDIR)/dpio2-drv-core.o

#
FLASH_LOADER = 	$(LIBDIR)/flash-loader

# Object file containig the vmosa x86-linux user space code.
X86_LINUX = 	$(LIBDIR)/dpio2-linux-user-space.o

#
INTEL_31154_P2P = $(LIBDIR)/intel-31154-p2p-optimal-set



# The the driver core is contained in these files.
DRIVER_CORE_OBJS =	vmosaCControlBit.o \
			vmosaCControlBitField.o \
			vmosaCControlDualByte.o \
			vmosaCControlQuadByte.o \
			vmosaCControlRegister.o \
			vmosaCDpio2DmaController.o \
			CDpio2Fifo.o \
			CDpio2FrontEnd.o \
			CDpio2Leds.o \
			CDpio2Oscillator.o \
			CDpio2OscillatorCY22150.o \
			vmosaCInterruptManager.o \
			CInterruptSource.o \
			vmosaCInterruptStatusRegister.o \
			vmosaCStatusBit.o \
			vmosaCStatusBitField.o \
			CAction.o \
			CDpio2DriverController.o \
			CDpio2DriverState.o \
			vmosaCDpio2HwInterface.o


DRIVER_LAYER_OBJS = 	dpio2Vmosa.o \
			dpio2Ioctl.o \
			CDpio2InterruptAction.o

FLASH_LOADER_OBJS = 	CDpio2Flash.o \
			SrecToBin.o \
			FlashLoaderForVmosa.o

X86_LINUX_OBJS =	dpio2-linux-user-space.o \
			vmosa-linux-user-space.o

INTEL_31154_P2P_OBJS =  set-intel-31154-p2p-prefetch-policy.o




#
# The Rules.
#

# set these to force evaluation.
.PHONY : clean build depend


# Build just the DPIO2 driver object file.
build : $(DRIVER) $(FLASH_LOADER) $(INTEL_31154_P2P)


# Build the driver
$(DRIVER) :   $(X86_LINUX) $(DRIVER_CORE) $(DRIVER_LAYER)
	make ensure_dir DIR=$(LIBDIR);
	$(LD) $(LDFLAGS) -o $@  $(X86_LINUX) $(DRIVER_CORE) $(DRIVER_LAYER)



$(X86_LINUX) : $(X86_LINUX_OBJS)
	make ensure_dir DIR=$(LIBDIR)
	$(LD) $(LDFLAGS) -o $@ $(X86_LINUX_OBJS)

$(DRIVER_CORE) : $(DRIVER_CORE_OBJS)
	make ensure_dir DIR=$(LIBDIR)
	$(LD) $(LDFLAGS) -o $@ $(DRIVER_CORE_OBJS)

$(DRIVER_LAYER) : $(DRIVER_LAYER_OBJS)
	make ensure_dir DIR=$(LIBDIR)
	$(LD) $(LDFLAGS) -o $@ $(DRIVER_LAYER_OBJS)

$(FLASH_LOADER) : $(FLASH_LOADER_OBJS)
	make ensure_dir DIR=$(LIBDIR)
	$(CPP) -pthread -static -o  $(FLASH_LOADER) $(DRIVER) $(FLASH_LOADER_OBJS)

$(INTEL_31154_P2P) : $(INTEL_31154_P2P_OBJS)
	make ensure_dir DIR=$(LIBDIR)
	$(CPP) -pthread -static -o  $(INTEL_31154_P2P) $(DRIVER) $(INTEL_31154_P2P_OBJS)

%.o : ./%.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o : $(PRJ_HOME)/src/core/%.cpp
	$(CPP) $(C++FLAGS) -o $@ -c $<

%.o : $(PRJ_HOME)/src/driver/%.cpp
	$(CPP) $(C++FLAGS) -o $@ -c $<

%.o : $(PRJ_HOME)/src/flash/%.cpp
	$(CC) $(C++FLAGS) -o $@ -c $<

%.o : $(PRJ_HOME)/src/flash/%.c
	$(CC) $(CFLAGS) -o $@ -c $<



# Clean 
clean :
	rm -r	$(DRIVER) \
		$(DRIVER_LAYER) \
		$(DRIVER_CORE) \
		$(X86_LINUX) \
		$(FLASH_LOADER) \
		$(INTEL_31154_P2P) \
		./*.o 


ensure_dir:
	@if test ! -d $(DIR) ; then \
	echo "Create" $(DIR);     \
	mkdir -p $(DIR); \
	fi
