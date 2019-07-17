/*****************************************************************************

	bmc_mdlib.h				Boston Micromachines Corp mirror driver library API

	Oct 16,		2014		CIW/SJT original entry

	© Copyright 2014, Boston Micromachines, Inc. All rights reserved.

*****************************************************************************/

/*==========================================================================*/
#ifndef	__BMC_MDRV_LIB__
#define	__BMC_MDRV_LIB__	1
/*==========================================================================*/
#ifdef	__cplusplus
extern "C" {
#endif
/*==========================================================================*/
/*
**	Data types used by the library
*/
typedef int						tE;		/* generic error return */
typedef unsigned int			tU32;	/* generic count/data (32b) */
typedef unsigned short			tU16;	/* generic data (16b) */
typedef unsigned char			tU8;	/* generic mask (0 or 1) */
typedef double					tD;		/* generic double */

typedef struct tBMClibStruct	*tBMC;	/* anonymous token for access */
/*
**	NOTE:	Since linux arrays are 0-based the actuators are number from 0.
*/
/*--------------------------------------------------------------------------*/
typedef enum {
	kBMCEnoErr		= 0,				/* no error */
	kBMCEbnf		= 12,				/* board not found */
	kBMCEversErr	= 1000,				/* hardware version mismatch */
	kBMCEunknownErr	= 1001,				/* unknown HVA */
	kBMCEorientErr	= 1002,				/* invalid orientation */
	kBMCEbadOrienErr= 1003,				/* unknown DM orientation */
	kBMCEbadLUTErr	= 1004,				/* invalid framed LUT */
	kBMCEnoEOFErr	= 1005,				/* no EOF in framed LUT */
	kBMCELUTErr		= 1006,				/* invalid HVA LUT */
	kBMCEactNumErr	= 1007,				/* actnum invalid for HVA */
	kBMCEbadDlenErr	= 1008,				/* dither len does not match mode */
	kBMCEbadLenErr	= 1010,				/* dither len > 2048 */
	kBMCEbadValErr	= 1011,				/* dither value outside 0.0-1.0 */
	kBMCEbadDrateErr= 1012,				/* dither rate must be >0 */
	kBMCEbadSeqErr	= 1013,				/* seq len must be >0, <=4096 */
	kBMCErangeErr	= 1014,				/* parameter is out of range */
	kBMCEdelayErr	= 1015,				/* seq delay must >0, <0.25 */
	kBMCEbadSrateErr= 1016,				/* sequence rate must be >0 */
	kBMCEbadPulseErr= 1017,				/* pulse must be 0..700 uSec */
	kBMCEbadChErr	= 1018,				/* DAC chan must be 0..1023 */
	kBMCEbadCmdErr	= 1019,				/* DAC cmd must be 0..65535 */
	kBMCEmodeErr	= 1020,				/* not in correct HVA mode */

	kBMCEnullErr	= 1100,				/* unexpected NULL parameter */
	kBMCEioErr,							/* ioctl() failed */
	kBMCEmemoryErr,						/* memory allocation failure */
	kBMCEbadTokenErr,					/* bad tBMC */
	kBMCEbadSpec,						/* invalid HVA spec */
	kBMCEbadZeroSpec,					/* spec active_map mismatch w/lut */
	kBMCEdupEntrySpec,					/* spec hva_lut duplicate entry */
	kBMCEbadTermSpec					/* no hva_lut/active_map term entry */
	} tBMCerrorEnum;
/*==========================================================================*/
/*
**	NOTE:	The BMC linux library uses 0-based identifiers for actuators.
**
**	EXCEPTION:	The tBMCHVAspec uses field minLUT to determine if the
**				actuator hva_lut table is 0-based or 1-based.
*/
/*==========================================================================*/
/*
**	Count the number of BMC mirror driver boards installed.
*/
tU32	BMCgetNumBoards(void);			/* return count of installed boards */
/*--------------------------------------------------------------------------*/
/*
**	Get the version strings for the library, driver, and hardware
**
**	NOTE:	Returned string is static store -- do not modify.
*/
char	*BMCgetLibRev(void);
char	*BMCgetDrvRev(void);
char	*BMCgetHWRev(void);
/*--------------------------------------------------------------------------*/
/*
**	Open and close a mirror driver board.
*/
tE	BMCopen(tU32 boardNdx, tBMC *bmc);	/* open a board */
tE	BMCclose(tBMC bmc);					/* close a board */
/*--------------------------------------------------------------------------*/
/*	Setup/configuration functions */
/*--------------------------------------------------------------------------*/
tE	BMCabortFrameData(tBMC bmc);
tE	BMCclearHVA(tBMC bmc);				/* reset all actuators to 0 */
tE	BMCdisableDither(tBMC bmc);
tE	BMCdisableSequence(tBMC bmc);
tE	BMCenableDither(tBMC bmc, tD frameRate);
tE	BMCenableSequence(tBMC bmc, tD frameRate);
tE	BMCpokeDM(tBMC bmc, tU16 actNum, tU16 data);	/* actNum is zero based */
typedef enum {							/* well-known HVA types */
	kBMCT_HVA32		= 0,
	kBMCT_HVA140,
	kBMCT_SD1024,
	kBMCT_KILO1024,
	kBMCT_HVA137,
	kBMCT_KILO492,
	kBMCT_KILO952,
	kBMCT_KILO2048,
	kBMCT_KILOLED,
	kBMCT_HVA4096,
	kBMCT_HVACOMPACT140,
	kBMCT_KILO4096LED_not_used_11,
	kBMCT_KILO2x952,
	kBMCT_KILOLongStroke,
	kBMCT_SD952,
	kBMCT_KILOS492,
	kBMCT_SDLED,
	kBMCT_KILO492C,
	kBMCT_KILO1024LL,
	kBMCT_KILO952LL,
	/*
	**	20..27 reserved
	*/
	kBMCT_firstUser	= 28,				/* first user defined idnum */
	kBMCT_lastUser	= 31,				/* max user defined idnum */
	kBMCT_maxSpec,						/* 32 */

	kBMCmarkLUT		= 0xDEAF,			/* after last hva_lut entry */
	kBMCmarkIAM		= 0xDB,				/* after last active_map */
	
	kBMCmaxSeqLen	= 4096,				/* hardware limit */
	kBMCmaxDithLen	= 2048				/*	ditto */
	} tBMC_HVAType_enum;

typedef struct {
	int				idnum;				/* used only by software */
	int				use_camlink;		/* 0 or 1 */
	int				use_fiber;			/* 0 or 1 */
	int				fiber_mode;			/* 0==S-Driver, 1==KILO */
	int				burst_mode;			/* 0 or 1; only 0 supported */
	int				sizeX;
	int				sizeY;				/* sizeX * sizeY <= 4096 */
	int				minLUT;				/* ID of 1st DAC in lUT (0 or 1) */
	tU16			*hva_lut;			/* sizeX * sizeY entries (1-based) */
										/* plus end entry of kBMCmarkLUT */
	tU8				*active_map;		/* sizeX * sizeY entries of 0/1 */
										/* plus end entry of kBMCmarkIAM */
	char			*description;		/* handy */
	} tBMCHVAspec, *tBMCHVAsp;

typedef enum {							/* mirror orientations */
	kBMCOnormal		= 0,
	kBMCOflipX,
	kBMCOflipY,
	kBMCOflipXY,
	kBMCOtranspose,
	kBMCOtransposeFlipX,
	kBMCOtransposeFlipY,
	kBMCOtransposeFlipXY
	} tBMC_orientation_enum;
tE	BMCsetDMorientation(tBMC bmc,
		tBMC_HVAType_enum HVAType, tBMC_orientation_enum dmOrientation);
tE	BMCsetHVA(tBMC bmc, tU16 data);	/* set all actuators to same value */
tE	BMCsetUpHVA(tBMC bmc, tBMC_HVAType_enum HVAType);
tE	BMCaddCustomHVA(tBMC bmc, tBMCHVAsp hvaSpec);
tE	BMCwriteDitherGains(tBMC bmc, tU32 count, tU16 *data);
tE	BMCwriteDitherWave(tBMC bmc, tU32 count, tD *data);	/* data 0.0 to 1.0 */
/*
**	v0 is the value in the LUT associated w/the DACid of 0 (should be 0 or 1)
*/
tE	BMCwriteHVALUT(tBMC bmc, tU32 count, tU16 *lut, tU8 *activeMap, int v0);
tE	BMCwriteSequence(tBMC bmc,
		tU32	nrows,
		tU32	ncols,
		tU32	nframes,
		tU16	*seq,				/* len = nframes * (# nonzero mask) */
		tU8		*mask,				/* len == nrows*ncols */
		tD		delaySec			/* 0.25 sec max */
		);
/*--------------------------------------------------------------------------*/
/*	Status functions */
/*--------------------------------------------------------------------------*/
tE	BMCgetCurrentHVAmode(tBMC bmc, tBMC_HVAType_enum *mode);
tE	BMCreadDMorientation(tBMC bmc, tBMC_orientation_enum *orientation);

typedef struct {
	tU32			reg;

	struct {
	unsigned int	SFP_nPRST		: 1;	/* low: SFP module present */
	unsigned int	SFP_TXF			: 1;	/* set: SFP has TX fault */
	unsigned int	SFP_LOS			: 1;	/* set: SFP lost light in fiber */
	unsigned int	CH_UP			: 1;	/* set: startup handshake ok */
	unsigned int	LN_UP			: 1;	/* set: startup handshake ok */
	unsigned int	Soft_ERR		: 1;	/* set: bit err or inv char */
	unsigned int	Hard_ERR		: 1;	/* set: hw fail or loss of clk */
	unsigned int	_reserved		: 25;
	} status;

	} tBMCfiberStatusStruct;
tE	BMCreadFiberLinkState(tBMC bmc, tBMCfiberStatusStruct *status);

typedef union {
	tU32			reg;

	struct {
	unsigned int	status0			: 1;	/* set: hi voltage on */
	unsigned int	status1			: 1;	/* set: data dropped (brief) */
	unsigned int	status2			: 1;	/* set: DACs busy (brief) */
	unsigned int	status3			: 1;	/* set: any err since pwr on */
	unsigned int	_reserved		: 28;
	} status;

	} tBMCHVAccStruct;
tE BMCreadHVAccStatus(tBMC bmc, tBMCHVAccStruct *status);

typedef enum {
	kBMCinvalidMap	= 1 << 16,		/* bit set then HVADAC does not exist */
	kBMCLUTlen		= 4096			/* size of returned data array */
	} tBMCHVALUTenum;

tE BMCreadHVALUT(tBMC bmc, tU32 *data);		/* 4k of data returned */

tE BMCreadHVAstate(tBMC bmc, tU32 len, tU32 *count, tU32 *data);

typedef union {
	tU32			reg;

	struct {
	unsigned int	DACovf			: 1;	/* set: DAC ch wrtn too quickly */
	unsigned int	ADDRoor			: 1;	/* set: bad addr was rcvd */
	unsigned int	HF				: 1;	/* set: Kilo FIFO half full */
	unsigned int	_reserved		: 29;
	} status;

	} tBMCkiloStatusStruct;
tE BMCreadKiloStatus(tBMC bmc, tBMCkiloStatusStruct *status);
tE BMCreadSequenceRate(tBMC bmc, tD *rate);
/*
**	Get parameters for well-known HVATypes (and custom types).
**
**	NOTE:	pointer is to static store -- do not modify!
*/
tE BMCretrieveHVAinfo(tBMC bmc, tBMC_HVAType_enum type, tBMCHVAsp *specPtr);

char *BMCgetErrStr(tE err);				/* returns str from static store */
/*--------------------------------------------------------------------------*/
/*	Runtime function */
/*--------------------------------------------------------------------------*/
#define	BMCburstHVA(bmc,count,data)	BMCwriteBurst(bmc,0x80,count,data)
/*--------------------------------------------------------------------------*/
/*	Test function */
/*--------------------------------------------------------------------------*/
typedef enum {
	kBMCTestAnyCmd		= 0,			/* cmd/actuator ignored */
	kBMCTestSpecificActuator,			/* cmd ignored */
	kBMCTestSpecificCommand,			/* actuator ignored */
	kBMCTestSpecificActuatorCommand,	/* actuator + cmd */
	kBMCTestFVAL,						/* pulse when FVAL is high */
	kBMCTestInternalErr					/* DAV is hi w/o FVAL */
	} tBMC_test_enum;
/*
**	TTL pulse is 16nS (0 uSecs) to 700 uSecs
*/
tE	BMCtestTTLout(tBMC bmc,
		tBMC_test_enum test, tU16 actuator, tU16 cmd, tD pulseUsec);
/*--------------------------------------------------------------------------*/
/*	Miscellaneous functions */
/*--------------------------------------------------------------------------*/
char *BMCgetOrienStr(tBMC_orientation_enum  orien);
/*==========================================================================*/
/*	Low-level access to hardware resources */
/*==========================================================================*/
/*
**	Used to slow down driver burst writes (if needed).
*/
tE	BMCsetSpinDelay(tBMC bmc, tU32 delayCount);
/*--------------------------------------------------------------------------*/
/*
**	Read and write a single 32b register.
*/
tE	BMCreadReg(tBMC bmc, tU32 regNdx, tU32 *value);
tE	BMCwriteReg(tBMC bmc, tU32 regNdx, tU32 value);
/*--------------------------------------------------------------------------*/
/*
**	Write 32b words to the burst window
**		addr is usually 0x80
**		count is the number of 16b words to write
**			NOTE: count _must_ be even
**		data[n] is the value for the nth actuator
*/
tE BMCwriteBurst(tBMC bmc, tU32 addr, tU32 count, tU16 *data);
/*
**	Be explicit that addr is a windown, not a register
*/
#define	BMCburstHVA(bmc,count,data)	BMCwriteBurst(bmc,0x80,count,data)
/*==========================================================================*/
#ifdef	__cplusplus
}
#endif
/*==========================================================================*/
#endif	/* __BMC_MDRV_LIB__ */
/*==========================================================================*/

