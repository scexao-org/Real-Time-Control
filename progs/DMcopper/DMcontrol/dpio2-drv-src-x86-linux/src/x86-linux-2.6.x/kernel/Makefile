############################################################################
#
# Makefile
#
# Revision History:
#  01b,03des2004,meh  Added support for cross compile.
#  01a,04nov2004,meh  Created.
#
############################################################################

# Compile on target
# make build

# Cross compile
# make build ARCH=i386 CROSS_COMPILE="path to toolchain"
# Set LINUX_SRC = <path to kernel src>

TARGET_PLATFORM = x86-linux-2.6.x

LINUX_SRC = /lib/modules/$(shell uname -r)/build

HERE = $(PWD)
PRJ_HOME = $(HERE)/../../..
LIB_DIR = $(PRJ_HOME)/lib/$(TARGET_PLATFORM)

MODULE_NAME = dpio2-linux-kernel-module
MODULE_OBJS = $(MODULE_NAME).o

EXTRA_CFLAGS += -I $(PRJ_HOME)/include
EXTRA_CFLAGS += -I $(PRJ_HOME)/src
EXTRA_CFLAGS += -I $(PRJ_HOME)/src/$(TARGET_PLATFORM)/include
EXTRA_CFLAGS += -D VM_LINUX

EXTRA_CFLAGS += -Wno-error=date-time 

ifneq ($(KERNELRELEASE),)
obj-m += $(MODULE_OBJS)
else	
KDIR:= $(LINUX_SRC)
PWD:=$(shell pwd)
endif

build:
	make ensure_dir DIR=$(LIB_DIR);
	$(MAKE) -Wall -C $(KDIR)  M=$(PWD) modules
	cp $(MODULE_NAME).ko $(LIB_DIR)/$(MODULE_NAME).ko
	cp $(MODULE_NAME).ko ../../../../rh-test-src/

clean:
	rm	$(MODULE_NAME).o \
		$(MODULE_NAME).mod.o \
		$(MODULE_NAME).mod.c \
		$(MODULE_NAME).ko \
		$(LIB_DIR)/$(MODULE_NAME).ko \

ensure_dir:
	@if test ! -d $(DIR) ; then \
	echo "Create" $(DIR);     \
	mkdir -p $(DIR); \
	fi
