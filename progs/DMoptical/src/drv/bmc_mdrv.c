/*****************************************************************************

	bmc_mdrv.c				Boston Micromachines Corp mirror driver

	Sep 28,		2014		CIW/SJT original entry

	© Copyright 2014, Boston Micromachines Corporation. All rights reserved.

	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    See <http://www.gnu.org/licenses/> for a copy of the license.

    IN NO EVENT WILL BMC BE LIABLE FOR ANY CONSEQUENTIAL, INCIDENTAL, OR
    SPECIAL DAMAGES, INCLUDING ANY LOST PROFITS OR LOST SAVINGS.

	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	$Author$

	$Date$

	$Id$

	Tab stops are 4.

*****************************************************************************/

/*==========================================================================*/
/*
**	Use the official version macros
*/
#include	<linux/version.h>

#include	<linux/module.h>
#include	<linux/init.h>
#include	<linux/pci.h>
#include	<linux/string.h>
#include	<linux/cdev.h>
#include	<linux/jiffies.h>

#include	<linux/fcntl.h>
#include	<linux/workqueue.h>
#include	<linux/sched.h>

#include	<linux/proc_fs.h>
#include	<linux/ioctl.h>
#include        <linux/uaccess.h>

#include	"bmc_mdrv.h"
/*--------------------------------------------------------------------------*/
#if	!defined(__i386__) && !defined(__x86_64__)
#error	This driver is only coded for 32b/64b Intel/AMD processors.
#endif
/*
**	These macros display strings in the kernel log, e.g., /var/log/syslog
*/
#define		SHOW(x)		{ printk x ; }
#define		ERR(x)		{ SHOW(("ERR: ")); SHOW(x); }
#define		DB(x)		if (dp->debug > 0) { SHOW(("DB: ")); SHOW(x); }
#define		DBV(x)		if (dp->debug > 1) { SHOW(("DBV: ")); SHOW(x); }
/*--------------------------------------------------------------------------*/
/*
**	Parameters which can be set when module is loaded.
*/
int		gDebug=0;					/* non-zero for early debugging */
int		gMsec=1000;					/* peridoc timer freq */
int		gSpin=0;					/* spin count for rd/wr delay */
/*
**	Export "debug", "mSec", and "spinCount"
*/
module_param_named(debug, gDebug, int, 0444);
MODULE_PARM_DESC(debug,"logging: 0 ERRs, 1 basicDB, 2 moreDB, 3 verboseDB");
module_param_named(mSec, gMsec, int, 0444);
MODULE_PARM_DESC(mSec,"Timer delay; default 1 sec");
module_param_named(spinCount, gSpin, int, 0444);
MODULE_PARM_DESC(spinCount,"Rd/Wr spin delay; default 0");
/*==========================================================================*/
static int mdrv_probe(struct pci_dev *dev, const struct pci_device_id *id);
static void mdrv_remove(struct pci_dev *dev);
/*
**	Hardware info for this PCI card
*/
static const struct pci_device_id	mdrv_ids[]={
	{
		.class =			0,
		.class_mask =		0,
		.vendor =			kBMCMDRVvendorID,
		.device =			kBMCMDRVdeviceID,
		.subvendor =		PCI_ANY_ID,
		.subdevice =		PCI_ANY_ID,
		.driver_data =		0
	},
	{ /* End: all zeros */ }
	};
/*
**	How we hook into linux PCI subsystem
*/
static struct pci_driver mdrv_driver = {
	.name			= kBMCMDRVname,
	.id_table		= mdrv_ids,
	.probe			= mdrv_probe,
	.remove			= mdrv_remove,
	.suspend		= NULL,
	//.suspend_late	= NULL,
	//.resume_early	= NULL,
	.resume			= NULL,
	.shutdown		= NULL
	};
/*==========================================================================*/
/*
**	Define a delay function prototype.
*/
typedef int (*tSpinFunc)(void *ptr);
/*--------------------------------------------------------------------------*/
/*
**	There is one of these allocated per installed board
*/
typedef struct {
	struct pci_dev		*dev;			/* how we hook back in to PCI */
	struct cdev			cdev;			/* how we hook back in to /dev */
	struct mutex		mutex;			/* board mutex */
	struct device		*device;		/* another piece of /dev */

	char				*barAddr;		/* mapped address of BAR */
	int					barLen;			/* size of resource */

	int					minorID;		/* device minor ID */
	int					state;			/* board state */
	int					workCount;		/* number of calls to work func */
	int					openCount;		/* count of driver clients */
	int					ioctlCount;		/* count of i/o calls */
	
	int					debug;			/* for this board */
	int					spinCount;		/*		ditto */
	/*
	**	For per-word rd/wr delay
	*/
	int					spinTarg;
	int					spinSpin;
	int					spinNdx;
	tSpinFunc			spinFunc;
	/*
	**	Local buffer for user data: used in burst write
	*/
	int					buff[kMaxCount>>1];
	} tMDRVstruct, *tDP;
/*--------------------------------------------------------------------------*/
/*
**	Variables global to this driver (not exported).
*/
static struct mutex		gMutex;			/* global mutex */
static dev_t			gMajor;			/* dynamically allocated by kernel */
static int				gMinorID=0;		/* increments for each board */
static struct class		*gClass;		/* dynamically created */
/*--------------------------------------------------------------------------*/
static int DoSpin(void *ptr)
/*
**	This function is accessed via pointer -- that assures us that gcc does
**	not optimize it into inline code.
*/
{
tDP	dp=(tDP)ptr;

	return(dp->spinSpin + 1);
}
/*
**	This macro is used to provide a delay between each word of a block read
**	or a block write.
*/
#define	SPIN_DELAY(dp)													\
	for (dp->spinNdx=0; dp->spinNdx < dp->spinCount; dp->spinNdx++) {	\
		dp->spinTarg += (dp->spinFunc)(dp); }
/*--------------------------------------------------------------------------*/
static int	mdrv_open(struct inode *i, struct file *f)
/*
**	Someone is opening the device file ("/dev/bmc_mdrvN")
**
**	Store our device pointer in the usual place
*/
{
tDP	dp;

	dp = container_of(i->i_cdev,tMDRVstruct,cdev);

	DB(("mdrv_open: entry with %p\n",dp));

	mutex_lock(&(dp->mutex));
	dp->openCount += 1;
	f->private_data = dp;
	mutex_unlock(&(dp->mutex));

	return(0);
}
/*--------------------------------------------------------------------------*/
static int	mdrv_close(struct inode *i, struct file *f)
/*
**	Someone is closing the /dev/bmc_mdrvN file
*/
{
tDP	dp;

	dp = container_of(i->i_cdev,tMDRVstruct,cdev);

	DB(("mdrv_close: entry with %p\n",dp));

	mutex_lock(&(dp->mutex));
	dp->openCount -= 1;
	f->private_data = NULL;
	mutex_unlock(&(dp->mutex));

	return(0);
}
/*--------------------------------------------------------------------------*/
#define	GET_ARGS(_struct_)	\
if (0 != copy_from_user(&_struct_,((void __user *)arg),sizeof(_struct_))) { \
	ret = -EFAULT; goto andOut; }

#define	PUT_ARGS(_struct_)	\
if (0 != copy_to_user(((void __user *)arg),&_struct_,sizeof(_struct_))) { \
	ret = -EFAULT; goto andOut; }

static long mdrv_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
/*
**	This is the heart of the driver: all control and i/o is via ioctl()
*/
{
tDP					dp;
int					ret=0;
tBMCMDRVinfoStruct	is;
tBMCMDRVctrlStruct	cs;
tBMCMDRVrwStruct	rws;
int					*ptr,i,*qtr,x;

	dp = (tDP)(f->private_data);
	/*
	**	Singulate all driver operations on this board.
	*/
	mutex_lock(&(dp->mutex));
	dp->ioctlCount += 1;
	switch (cmd) {
	  case BMCMDRVINFO:
		/*
		**	Return information about the driver and state of same
		*/
		DB(("mdrv_ioctl: info call\n"));
		is.majorVersion = kBMCMDRVmajorVers;
		is.minorVersion = kBMCMDRVminorVers;
		is.debug = dp->debug;
		is.spinCount = dp->spinCount;
		is.state = dp->state;
		is.openCount = dp->openCount;
		is.ioctlCount = dp->ioctlCount;
		PUT_ARGS(is);
		break;
	  case BMCMDRVCTRL:
		/*
		**	Set driver-specific parameters
		*/
		GET_ARGS(cs);
		DB(("mdrv_ioctl: ctrl call\n"));
		dp->debug = cs.debug;			/* debug display */
		dp->spinCount = cs.spinCount;	/* per word rd/wr delay */
		break;
	  case BMCMDRVREAD:
		/*
		**	Read a block of data from the register space.
		*/
		GET_ARGS(rws);
		if ((rws.ndx < 0) || ((rws.ndx + (rws.incr * rws.count)) > 24)) {
		  ERR(("mdrv_ioctl: read %d from %d is out of range\n",
			rws.count,rws.ndx));
		  ret = -ERANGE;
		  break;
		  }
		DB(("mdrv_ioctl: read call (%d ndx, %d count, %d incr)\n",
			rws.ndx,rws.count,rws.incr));
		ptr = (int *)dp->barAddr;
		ptr += rws.ndx;
		for (i=0,qtr=rws.data; i<rws.count; i++) {
		  x = *ptr;					/* move PCI to local RAM */
		  /*
		  **	Move from local RAM to user space
		  */
		  if (0 != copy_to_user((void __user *)qtr,&x,sizeof(int))) {
			ERR(("mdrv_ioctl: copy_to_user failed at %d\n",i));
			ret = -EFAULT;
			break;
			}
		  qtr += 1;
		  ptr += rws.incr;
		  SPIN_DELAY(dp);
		  }
		break;
	  case BMCMDRVWRITE:
		/*
		**	Write a block of data to the register space.
		*/
		GET_ARGS(rws);
		if ((rws.ndx < 0) || ((rws.ndx + (rws.incr * rws.count)) > 2079)) {
		  ERR(("mdrv_ioctl: write %d to %d is out of range\n",
			rws.count,rws.ndx));
		  ret = -ERANGE;
		  break;
		  }
		DB(("mdrv_ioctl: write call (%d ndx, %d count, %d incr)\n",
			rws.ndx,rws.count,rws.incr));
		ptr = (int *)dp->barAddr;
		ptr += rws.ndx;
		/*
		**	This copies in all user data w/1 call: max speed for burst.
		*/
		qtr = rws.data;
		if (0!=copy_from_user(dp->buff,(void __user *)qtr,rws.count*sizeof(int))) {
		  ERR(("mdrv_ioctl: copy_from_user of %d failed\n",rws.count));
		  ret = -EFAULT;
		  break;
		  }
		for (i=0,qtr=rws.data; i<rws.count; i++) {
		  *ptr = dp->buff[i];			/* move from local RAM to PCI */
		  qtr += 1;
		  ptr += rws.incr;
		  SPIN_DELAY(dp);
		  }
		break;
	  default:
		ERR(("mdrv_ioctl: do not know 0x%08X\n",cmd));
		ret = -EINVAL;
	  }

andOut:
	/*
	**	Be sure to release the lock
	*/
	mutex_unlock(&(dp->mutex));

	return(ret);
}
/*--------------------------------------------------------------------------*/
/*
**	How we hook into the linux file abstraction
*/
static const struct file_operations sfops = {
	.owner =			THIS_MODULE,
	.open =				mdrv_open,
	.release =			mdrv_close,
	.unlocked_ioctl =	mdrv_ioctl
	};
/*==========================================================================*/
static int mdrv_probe(struct pci_dev *dev, const struct pci_device_id *id)
/*
**	This function is called by the kernel when it sees a PCI card with our
**	vendorID.
**
**	Here we hook into the kernel.  We would also init the device if that were
**	required.
*/
{
int	ret=-ENODEV;						/* preset for failure */
tDP	dp=NULL;
	/*
	**	Block any other card from initializing; not really necessary but
	**	always safe.
	*/
	mutex_lock(&gMutex);
	/*
	**	Get our private control structure.
	*/
	if (NULL == (dp = (tDP)kzalloc(sizeof(tMDRVstruct),GFP_KERNEL))) {
	  ret = -ENOMEM;
	  goto andOut;
	  }
	dp->debug = gDebug;
	dp->spinCount = gSpin;
	dp->spinFunc = DoSpin;

	DB(("mdrv_probe: initializing %d\n",gMinorID));

	dp->dev = dev;
	dp->minorID = gMinorID++;
	dp->state = -1;
	mutex_init(&(dp->mutex));
	pci_set_drvdata(dev,dp);
	/*
	**	Now hook into /dev
	*/
	if (NULL == (dp->device = device_create(gClass,NULL,
			MKDEV(MAJOR(gMajor),dp->minorID),dp,"bmc_mdrv%d",dp->minorID))) {
	  ERR(("device_create failed\n"));
	  goto errOut;
	  }

	cdev_init(&(dp->cdev),&sfops);
	dp->cdev.owner = THIS_MODULE;
	dp->cdev.ops = &sfops;
	if (0 > cdev_add(
			&(dp->cdev),MKDEV(MAJOR(gMajor),dp->minorID),1)) {
	  ERR(("cdev_add() failed\n"));
	  goto errOut;
	  }
	/*
	**	Begin board interaction
	**
	**	NOTE: we do NOT call pci_set_master() since there is no DMA capability.
	*/
	if (0 > pci_enable_device(dp->dev)) {
	  ERR(("pci_enable_device() failed\n"));
	  cdev_del(&(dp->cdev));
	  goto errOut;
	  }
	/*
	**	Get the PCI resources.
	*/
	if (0 != pci_request_regions(dp->dev,kBMCMDRVname)) {
	  ERR(("pci_request_regions() failed\n"));
	  cdev_del(&(dp->cdev));
	  goto errOut;
	  }
	/*
	**	The lspci output shows there is only one BAR
	*/
	dp->barLen = pci_resource_len(dp->dev,0);
	if (NULL == (dp->barAddr = pci_iomap(dp->dev,0,dp->barLen))) {
	  ERR(("pci_iomap() failed\n"));
	  cdev_del(&(dp->cdev));
	  pci_release_regions(dp->dev);
	  goto errOut;
	  }
	/*
	**	We are done.
	*/
	SHOW(("mdrv_probe: success for %d/%p\n",dp->minorID,dp));

	dp->state = 0;

	ret = 0;

andOut:
	mutex_unlock(&gMutex);

	return(ret);

errOut:
	if (NULL != dp) {
	  if (NULL != dp->device) {
		device_destroy(gClass,MKDEV(MAJOR(gMajor),dp->minorID));
		}
	  kfree(dp);
	  }
	pci_disable_device(dev);

	goto andOut;
}
/*--------------------------------------------------------------------------*/
static void mdrv_remove(struct pci_dev *dev)
/*
**	This function completely removes a driver instance from the kernel.
**
**	There is no DMA and no interrupt capability.  This will not be called
**	if there are ny open file handles into this driver.
*/
{
tDP	dp;

	dp = (tDP)pci_get_drvdata(dev);
	/*
	**	Unhook from /dev
	*/
	cdev_del(&(dp->cdev));
	device_destroy(gClass,MKDEV(MAJOR(gMajor),dp->minorID));
	/*
	**	Remove the BAR mapping
	*/
	pci_iounmap(dp->dev,dp->barAddr);
	pci_release_regions(dev);
	/*
	**	And disable the hardware
	*/
	pci_disable_device(dev);

	SHOW(("mdrv_remove: %d is gone\n",dp->minorID));
	/*
	**	Clear and free our storage
	*/
	(void)memset(dp,'\0',sizeof(tMDRVstruct));
	kfree(dp);

	pci_set_drvdata(dev,NULL);

	return;
}
/*==========================================================================*/
static int __init driver_initf(void)
/*
**	The very first call the module sees when it is loaded into the kernel.
*/
{
	mutex_init(&(gMutex));				/* the global mutex */
	/*
	**	Get a dynamic major device number.
	*/
	if (0 > alloc_chrdev_region(&gMajor,0,kBMCMDRVmaxBoards,kBMCMDRVname)) {
	  ERR(("driver_initf: alloc_chrdev_region() failed\n"));
	  return(-1);
	  }
	/*
	**	Register our device class
	*/
	if (NULL == (gClass = class_create(THIS_MODULE,kBMCMDRVname))) {
	  ERR(("driver_initf: could not register class=\n"));
	  pci_unregister_driver(&mdrv_driver);
	  return(-1);
	  }
	/*
	**	Now let the PCI subsystem look for any BMC boards
	*/
	return(pci_register_driver(&mdrv_driver));
}
/*--------------------------------------------------------------------------*/
static void __exit driver_exitf(void)
/*
**	The very last call the driver module sees before it is unloaded.
*/
{
	/*
	**	First, unhook from the PCI subsystem
	*/
	pci_unregister_driver(&mdrv_driver);
	/*
	**	Remove the class
	*/
	class_destroy(gClass);
	/*
	**	Now give back our major device number
	*/
	unregister_chrdev_region(gMajor,kBMCMDRVmaxBoards);

	return;
}
/*==========================================================================*/
/*
**	These are the well-known entry/exit points for any kernel module.
*/
module_init(driver_initf);
module_exit(driver_exitf);
/*--------------------------------------------------------------------------*/
MODULE_AUTHOR("Cambridge Information Ware/SJT");
MODULE_DESCRIPTION("Boston Micromachines Corporation mirror driver");
MODULE_LICENSE("Dual BSD/GPL");
/*==========================================================================*/

