/*****************************************************************************

	bmc_mdrv.h				Boston Micromachines Corp mirror driver declarations

	Sep 28,		2014		CIW/SJT original entry

	© Copyright 2014, Boston Micromachines Corporation. All rights reserved.

	Tab stops are 4.

*****************************************************************************/

/*==========================================================================*/
#ifndef	__BMC_MDRV__
#define	__BMC_MDRV__	1
/*==========================================================================*/
typedef enum {
	kBMCMDRVmajorVers	= 2,			/* 2nd implementation */
	kBMCMDRVminorVers	= 1,			/* release sequence */

	kBMCMDRVvendorID	= 0x1A8F,
	kBMCMDRVdeviceID	= 0x2000,
	kBMCMDRVmaxBoards	= 16,			/* max number of boards in one PC */

	kBMCMDRVmagic		= 'm',			/* for ioctl() */
	/*
	**	Register numbers (from BMC_HVAIF_Manual Rev5X.docx)
	*/
	kRegHWrev			= 0,			/* RO FPGA Hardware Revision */
	kRegHVAmode,						/* RW HVA Mode */
	kRegRsrvd2,							/* -- */
	kRegHVAburstAbort,					/* WO HVA Burst Abort */
	kRegHVApoke,						/* RW HVA Poke; 31:16 act; 15:0 cmd */
	kRegHVAposition,					/* RO Current HVA position (15:0) */
	kRegHVALUT,							/* RW HVA LUT */
	kRegDMorientation,					/* RW DM Orientation (2:0) */
	kRegDitherPeriod,					/* RW Dither Period (16 nS counts) */
	kRegDitherWaveform,					/* RW RAM (2048 max len) */
	kRegDitherPattern,					/* RW Dither pattern */
	kRegRsrvd11,						/* -- */
	kRegMultiXstatus,					/* RO Multi-X Status (4:1) */
	kRegTest,							/* RW Test (31:0) */
	kRegRsrvd14,						/* -- */
	kRegFiberStatus,					/* RO Fiber Status */
	kRegHVActrl,						/* RW HVA Control */
	kRegSeqPeriod,						/* RW Sequence Period */
	kRegSeqControl,						/* RW Sequence Control */
	kRegSeqPatter,						/* RW Sequence Pattern */
	kRegSeqDelay,						/* RW Sequence Delay */
	kRegTestAddress,					/* RW Test Address */
	kRegTestSelect,						/* RW Test Select */
	kRegTTLstretch,						/* RW TTL Stretch */
	kRegKiloHVAstatus,					/* RO Kilo HVA Status */
	kRegKiloHVAstatus2	= 24,			/* just for checking enumerates */

	kMaxCount			= 4096,
	/*
	**	This is the burst window for all HVAs
	*/
	kBurstWindowFirst	= 32,			/* PCI offset 0x80 */
	kBurstWindowLast	= 2079			/* PCI offset 207C */
	} tBMCenum;

#define	kBMCMDRVname	"bmc_mdrv"
/*--------------------------------------------------------------------------*/
typedef union {
	int				reg;				/* 32b version */

	struct {
	unsigned int	minorRev	: 8;
	unsigned int	majorRev	: 8;
	unsigned int	designID	: 16;
	} r0;								/* 0x20000102 */

	struct {
	unsigned int	HVASel		: 5;	/* no control over the hardware */
	unsigned int	useCL		: 1;	/* max CameraLink actuators is 256 */
	unsigned int	useFiber	: 1;	/* S-driver 1024; Kilo-driver 4096 */
	unsigned int	fiberMode	: 1;	/* 0==S-drive; 1==Kilo-drive */
	unsigned int	_reserved	: 8;
	unsigned int	HVASize		: 16;	/* max is actually 4096 */
	} r1;

	struct {
	unsigned int	_reserved	: 31;
	unsigned int	Abort		: 1;	/* required if bad data burst count */
	} r3;

	struct {
	unsigned int	data		: 16;
	unsigned int	actNum		: 16;
	} r4;

	struct {							/* reset; read entire array; reset */
	unsigned int	data		: 16;
	unsigned int	_reserved	: 15;
	unsigned int	reset		: 1;	/* zeros internal addr */
	} r5;

	struct {							/* reset; read or write incrs addr */
	unsigned int	hvadac		: 12;	/* channel ID 0..N-1 */
	unsigned int	zero		: 4;	/* always set to zero */
	unsigned int	invalid		: 1;	/* e.g., 4 corners */
	unsigned int	_reserved	: 15;
	unsigned int	reset		: 1;	/* zeros internal addr */
	} r6;

	struct {							/* no affect on HW */
	unsigned int	orien		: 3;	/* -/fX/fY/fXY/tr/tr,fX/tr,fY/tr,fXY */
	unsigned int	_reserved	: 28;
	} r7;

	struct {
	unsigned int	ditherPeriod;		/* 16 nSec units; 0 disables dither */
	} r8;

	struct {							/* reset; read or write incrs addr */
	unsigned int	data		: 16;	/* ??? 0..32768 ??? */
	unsigned int	eow			: 1;	/* signals end-of-waveform */
	unsigned int	_reserved	: 15;
	unsigned int	reset		: 1;	/* zeros internal addr; 2..2048 elem */
	} r9;

	struct {
	unsigned int	actCmd		: 16;	/* 0..65535 */
	unsigned int	_reserved	: 15;
	unsigned int	reset		: 1;	/* zeros internal addr */
	} r10;

	struct {							/* CameraLink CC */
	unsigned int	status0		: 1;	/* set if CL lock; low on over temp */
	unsigned int	status1		: 1;	/* set when data written too fast */
	unsigned int	status2		: 1;	/* DAC busy: set for 0.6 uSec */
	unsigned int	status3		: 1;	/* latched error; cleared pwr cycle */
	unsigned int	_reserved	: 28;
	} r12;

	struct {
	unsigned int	test;				/* no affect on HW */
	} r13;

	struct {
	unsigned int	SFPnPRST	: 1;	/* 0 => SFP module present */
	unsigned int	SFPTXF		: 1;	/* 1 => SFP has TX fault */
	unsigned int	SFPLOS		: 1;	/* 1 => SFP has lost light */
	unsigned int	CHUP		: 1;	/* 1 => startup handshake completed */
	unsigned int	LNUP		: 1;	/* 1 => startup handshake completed */
	unsigned int	softERR		: 1;	/* 1 => bit error or invalid char */
	unsigned int	hardERR		: 1;	/* 1 => HW failure or loss of clk */
	unsigned int	_reserved	: 25;
	} r15;

	struct {
	unsigned int	data		: 16;
	unsigned int	addr		: 12;
	unsigned int	two			: 3;	/* _always_ "010" */
	unsigned int	cntl		: 1;	/* 0 => DAC reg; 1 => ctrl reg */
	} r16_Sdriver;

	struct {							/* only used in factory testing */
	unsigned int	data		: 16;
	unsigned int	actNum		: 12;
	unsigned int	zero		: 2;	/* 00 */
	unsigned int	dav			: 1;
	unsigned int	zero2		: 1;	/* 0 */
	} r16_KILOdriver;

	struct {
	unsigned int	seqPeriod;			/* 0 or units of 16 nSec */
	/*
	**	NOTE: if 0 is written TTL triggers; read shows current period of TTL
	*/
	} r17;

	struct {
	unsigned int	ena			: 1;	/* begin sequencing */
	unsigned int	_reserved	: 30;
	unsigned int	reset		: 1;	/* reset internal state machine */
	} r18;

	struct {							/* reset; rd/wr increments; 4096 sz */
	unsigned int	actCmd		: 16;	/* DAC cmd */
	unsigned int	actNum		: 12;	/* HVA address */
	unsigned int	eof			: 1;	/* 1 => end of frame */
	unsigned int	eop			: 1;	/* 1 => end of pattern */
	unsigned int	zero		: 1;	/* 0 */
	unsigned int	reset		: 1;	/* reset internal addr */
	} r19;

	struct {
	unsigned int	seqDelay	: 24;	/* delay aftr TLL trig; 16 nS units */
	unsigned int	zero		: 8;
	} r20;

	struct {
	unsigned int	actCmd		: 16;	/* TTL pulses when cmd seen */
	unsigned int	actNum		: 12;	/* TTL pulses when act written */
	unsigned int	zero		: 4;
	} r21;

	struct {
	unsigned int	testSel		: 4;	/*	0	any cmd to HVA
											1	any cmd to specific act
											2	specific cmd to any act
											3	specific cmd to specific act
											4	TTL hi when FVAL is high
											5	internal err (VAV w/o FVAL) */
	unsigned int	zero		: 27;
	unsigned int	stretch		: 1;	/* 0 => 16 nS pulse */
	} r22;

	struct {
	unsigned int	stretch		: 16;	/* 16 nSec units */
	unsigned int	zero		: 16;
	} r23;

	struct {
	unsigned int	DACOVF		: 1;	/* DAC channel written too quickly */
	unsigned int	ADDROVF		: 1;	/* bad address */
	unsigned int	HF			: 1;	/* Kilo FIFO is half full */
	unsigned int	_reserved	: 29;
	} r24;

	struct {
	unsigned int	cmdA		: 16;	/* odd HVA actuator cmd (1, 3, ...) */
	unsigned int	cmdB		: 16;	/* evn HVA actuator cmd (2, 4, ...) */
	} rBurst;

	} tRegIOstruct, *tRIOp;
/*--------------------------------------------------------------------------*/
typedef struct {						/* get info from driver */
	int					majorVersion;	/* driver software version */
	int					minorVersion;	/*		ditto */
	int					debug;			/* state of driver debug */
	int					spinCount;		/* per-word rd/wr delay */
	int					state;			/* state of driver */
	int					openCount;		/* number of clients */
	int					ioctlCount;		/* i/o cals */
	} tBMCMDRVinfoStruct, *tIOinfop;

typedef struct {						/* change driver setting */
	int					debug;			/* new driver debug setting */
	int					spinCount;		/* per-word rd/wr delay */
	} tBMCMDRVctrlStruct, *tIOctrlp;

typedef struct {						/* register block read or write */
	int					ndx;			/* register index */
	int					count;			/* number of 32b values to rd/wr */
	int					incr;			/* 0 to read/write at same addr */
	int					*data;			/* where to store/fetch the data */
	} tBMCMDRVrwStruct, *tIORWp;
/*
**	There are only 4 ioctl()
**		get driver info
**		set driver ctrl
**		read block from register w/optional increment between reads
**		write block to register w/optional increment between writes
**			NOTE: in general
**					count is 1 and incr is irrelevant
**					or
**					count is > 1 and incr is 0
*/
#define	BMCMDRVINFO		_IOR(kBMCMDRVmagic,1,tBMCMDRVinfoStruct)
#define	BMCMDRVCTRL		_IOW(kBMCMDRVmagic,2,tBMCMDRVctrlStruct)
#define	BMCMDRVREAD		_IOR(kBMCMDRVmagic,3,tBMCMDRVrwStruct)
#define	BMCMDRVWRITE	_IOW(kBMCMDRVmagic,4,tBMCMDRVrwStruct)
/*==========================================================================*/
#endif	/* __BMC_MDRV__ */
/*==========================================================================*/

