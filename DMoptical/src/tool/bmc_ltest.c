/*****************************************************************************

	bmc_ltest.c			Boston Micromachines Corp. library test program

	Nov 1,		2014	CIW/SJT original entry

	© Copyright 2014, Boston Micromachines Corporation. All rights reserved.

	Tab stops are 4.

*****************************************************************************/

/*==========================================================================*/
#include	<stdio.h>
#include	<stdarg.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>
#include	<math.h>
#include       <sys/time.h>

#include	"bmc_mdlib.h"
/*==========================================================================*/
static int	sDebug=0;						/* show debug output */
static int	sNdx=0;							/* board index */
static int	sExit=0;						/* program exit code */

static tBMC	sBMC=NULL;
/*--------------------------------------------------------------------------*/
#define	SHOW(x)	{ (void)printf x ; (void)fflush(stdout); }
#define	ERR(x)	{ SHOW(("ERR: ")); SHOW(x); }
#define	DB(x)	if (sDebug > 0) { SHOW(x); }
#define	DDB(x)	if (sDebug > 1) { SHOW(x); }

#define	kBIGDATA	4096
#define	kSMALLDATA	2048
/*--------------------------------------------------------------------------*/
static char	*sArgv0=NULL;					/* name of executable */
static void ShowHelp(void)
{
	SHOW(("%s of " __DATE__ " at " __TIME__ "\n",sArgv0));
	SHOW(("   arguments:\n"));
	SHOW(("   -h               display this message and exit\n"));
	SHOW(("   -d               display program debug output\n"));
	SHOW(("   -l str           display str in output\n"));
	/*
	**	No actual i/o
	*/
	SHOW(("   -b ndx           ndx of board (/dev/bmc_mdrv<n>) (default 0)\n"));
	SHOW(("   -u uSecs         delay for microseconds\n"));
	/*
	**	Low-level tests
	*/
	SHOW(("   -r ndx           read register ndx\n"));
	SHOW(("   -R ndx value     set register ndx to value\n"));
	SHOW(("   -B n a v0 inc    wr len n to addr a: v0, v0+inc,...\n"));
	SHOW(("   -a               abort frame data\n"));
	SHOW(("   -D count         set driver spin delay count\n"));
	/*
	**	Highest-level tests
	*/
	SHOW(("   -i               get nBoards, vers, HVAmode, orien, stat\n"));
	SHOW(("   -F               add 'F' HVAtype\n"));
	SHOW(("   -c id desc_str   add custom HVAtype w/id & description\n"));
	SHOW(("   -I type          initialize board for HVAtype\n"));
	/*
	**	Post init tests
	*/
	SHOW(("   -O type orien    setup board for HVAtype and orientation\n"));
	SHOW(("   -p act value     poke actuator w/value\n"));
	SHOW(("   -Z               reset all actuators to zero\n"));
	SHOW(("   -z value         set all actuators to value\n"));
	/*
	**	Sequence/dither tests
	*/
	SHOW(("   -S frameRate     enable sequence w/frameRate\n"));
	SHOW(("   -s               disable sequence\n"));
	SHOW(("   -W frameRate     enable dither w/frameRate\n"));
	SHOW(("   -w               disable dither\n"));
	SHOW(("   -j delay         setup sequence to rotate image w/delay\n"));
	SHOW(("   -J c|C           setup (c)oncave or (C)onvex dither\n"));
	/*
	**	Setup TTLout
	*/
	SHOW(("   -T t a c us	   TLL test t, act a, cmd c, pulse uSec us\n"));
	/*
	**	Data dump
	*/
	SHOW(("   -A               dump all\n"));
	/*
	**	Timing tests
	*/
	SHOW(("   -t nloops        test timing for i/o\n"));

	SHOW(("\n"));
	SHOW(("  bmc_ltest:        exercise BMC mirror driver lib per cmd line\n"));
	SHOW(("\n"));
}
/*==========================================================================*/
static void MakeOpen(void)
/*
**	Make sure the library has been opened
*/
{
tE	err;

	if (NULL != sBMC) { return; }

	if (kBMCEnoErr != (err = BMCopen(sNdx,&sBMC))) {
	  ERR(("MakeOpen: %d gave %s\n",sNdx,BMCgetErrStr(err)));
	  exit(1);
	  }

	return;
}
/*==========================================================================*/
static void DoInfo(void)
/*
**	Show basic information about the board state.
*/
{
tBMC_HVAType_enum		mode;
tBMC_orientation_enum	orien;
tBMCHVAsp				sp;
tBMCfiberStatusStruct	fs;
tBMCHVAccStruct			hs;
tBMCkiloStatusStruct	ks;
tE						err;
tD						dd;

	SHOW(("DoInfo: we detect %d boards\n",BMCgetNumBoards()));
	SHOW(("  lib rev: '%s'\n",BMCgetLibRev()));
	SHOW(("  drv rev: '%s'\n",BMCgetDrvRev()));
	SHOW(("  HW  rev: '%s'\n",BMCgetHWRev()));
	/*
	**	Remaining info applies to a specific board
	*/
	MakeOpen();

	if (kBMCEnoErr != (err = BMCgetCurrentHVAmode(sBMC,&mode))) {
	  ERR(("  BMCgetCurrentHVAmode gave '%s'\n",BMCgetErrStr(err)));
	  return;
	  }
	if (kBMCEnoErr != (err = BMCretrieveHVAinfo(sBMC,mode,&sp))) {
	  if ((kBMCEunknownErr == err) && (mode >= kBMCT_firstUser)) {
		SHOW(("  HVA:      %d is unknown at this time\n",mode));
		goto afterHVAinfo;
		} else {
		ERR(("  BMCretrieveHVAinfo gave '%s'\n",BMCgetErrStr(err)));
		return;
		}
	  }
	SHOW(("  HVA:      %d '%s'\n",mode,sp->description));
afterHVAinfo:

	if (kBMCEnoErr != (err = BMCreadDMorientation(sBMC,&orien))) {
	  ERR(("  BMCreadDMorientation gave '%s'\n",BMCgetErrStr(err)));
	  return;
	  }
	SHOW(("  orien:    %d '%s'\n",orien,BMCgetOrienStr(orien)));

	if (kBMCEnoErr != (err = BMCreadFiberLinkState(sBMC,&fs))) {
	  ERR(("  BMCreadFiberLinkState gave '%s'\n",BMCgetErrStr(err)));
	  return;
	  }
	SHOW(("  fiber:    0x%08X\n",fs.reg));

	if (kBMCEnoErr != (err = BMCreadHVAccStatus(sBMC,&hs))) {
	  ERR(("  BMCreadHVAccStatus gave '%s'\n",BMCgetErrStr(err)));
	  return;
	  }
	SHOW(("  HVAcc:    0x%08X\n",hs.reg));

	if (kBMCEnoErr != (err = BMCreadKiloStatus(sBMC,&ks))) {
	  ERR(("  BMCreadKiloStatus gave '%s'\n",BMCgetErrStr(err)));
	  return;
	  }
	SHOW(("  Kilo:     0x%08X\n",ks.reg));

	if (kBMCEnoErr != (err = BMCreadSequenceRate(sBMC,&dd))) {
	  ERR(("  BMCreadKiloStatus gave '%s'\n",BMCgetErrStr(err)));
	  return;
	  }
	SHOW(("  seqRate:  %.6lf\n",dd));

	return;
}
/*--------------------------------------------------------------------------*/
#define	CHUNK_SZ(sz)	(((sz + 7) >> 3) << 3)

static tBMCHVAsp CloneSpec(tBMCHVAsp sp, int newID, char *newStr)
/*
**	A useful tool: clone an existing HVA spec, changing idnum and description
**
**	NOTE: we allocate a single block for EZ free.
*/
{
int			sz=0;
tBMCHVAsp	ans;
tU16		*p16,*q16;
tU8			*p8,*q8;
char		*str;

	if (NULL == sp)		{ return(NULL); }
	if (NULL == newStr)	{ return(NULL); }

	sz = CHUNK_SZ(sizeof(tBMCHVAspec));
	sz += CHUNK_SZ(((sp->sizeX * sp->sizeY)+1) * sizeof(tU16));
	sz += CHUNK_SZ(((sp->sizeX * sp->sizeY)+1) * sizeof(tU8));
	sz += CHUNK_SZ(strlen(newStr) + 1);

	if (NULL == (ans = (tBMCHVAsp)malloc(sz))) { return(NULL); }
	(void)memset(ans,'\0',sz);

	p16 = (tU16 *)(((char *)ans) + CHUNK_SZ(sizeof(tBMCHVAspec)));
	p8 = (tU8 *)
		(((char *)p16) + CHUNK_SZ(((sp->sizeX * sp->sizeY)+1) * sizeof(tU16)));
	str = (char *)
		(((char *)p8) + CHUNK_SZ(((sp->sizeX * sp->sizeY)+1) * sizeof(tU8)));

	(void)memcpy(ans,sp,sizeof(tBMCHVAspec));
	ans->idnum = newID;
	ans->hva_lut = p16;
	ans->active_map = p8;
	ans->description = str;
	(void)memcpy(p16,sp->hva_lut,((sp->sizeX * sp->sizeY)+1) * sizeof(tU16));
	(void)memcpy(p8,sp->active_map,((sp->sizeX * sp->sizeY)+1) * sizeof(tU8));
	(void)strcpy(str,newStr);

	return(ans);
}
/*--------------------------------------------------------------------------*/
static void DumpSpec(tBMCHVAsp sp)
/*
**	Single place to dump any HVA spec
*/
{
int		i,j,n,m;
char	fmtA[16],fmtB[16];
char	line[kBIGDATA];
	/*
	**	Know our number sizing as we dump the spec
	*/
	n = sp->sizeX * sp->sizeY;
	if (n < 10) {
	  m = 1;
	  } else if (n < 100) {
	  m = 2;
	  } else if (n < 1000) {
	  m = 3;
	  } else {
	  m = 4;
	  }
	(void)sprintf(fmtA,"%%0%0dd",m);
	(void)sprintf(fmtB,"%%%ds",m);
	SHOW(("tBMCHVAsp: '%s'\n"
		  "           %d X %d; %d cl, %d fi, %d fm, %d bm (%d minLUT)\n",
		sp->description,sp->sizeX,sp->sizeY,sp->use_camlink,sp->use_fiber,
		sp->fiber_mode,sp->burst_mode,sp->minLUT));
	n = 0;
	for (i=0; i<sp->sizeY; i++) {
	  (void)sprintf(line,"  ");		/* new line */
	  for (j=0; j<sp->sizeX; j++) {
		if (0 == sp->active_map[n]) {
		  (void)sprintf(line+strlen(line),fmtB,"");
		  (void)strcat(line,",");
		  } else {
		  (void)sprintf(line+strlen(line),fmtA,sp->hva_lut[n]);
		  (void)strcat(line,",");
		  }
		n += 1;						/* next entry */
		}
	  SHOW(("%s\n",line));
	  }
	SHOW(("      term: 0x%08X -- 0x%02X --\n",
		sp->hva_lut[n],sp->active_map[n]));

	return;
}
/*--------------------------------------------------------------------------*/
static void DoAddF(void)
/*
**	Add an 'F' shaped HVTtype (to illustrate transpose/flip)
**
**	Also illustrates adding a custom HVAType
*/
{
static tU16 sHVAF_lut[]={
                0,0,0,0,0,0,0,0,
                0,0,1,2,3,0,0,0,
                0,0,4,0,0,0,0,0,
                0,0,5,6,0,0,0,0,
                0,0,7,0,0,0,0,0,
                0,0,8,0,0,0,0,0,
                0,0,9,0,0,0,0,0,
                0,0,0,0,0,0,0,0,
				kBMCmarkLUT
				};
static tU8 sHVAF_am[]={
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 1, 1, 1, 0, 0, 0,
				0, 0, 1, 0, 0, 0, 0, 0,
				0, 0, 1, 1, 0, 0, 0, 0,
				0, 0, 1, 0, 0, 0, 0, 0,
				0, 0, 1, 0, 0, 0, 0, 0,
				0, 0, 1, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				kBMCmarkIAM
				};
static tBMCHVAspec	sHVAF={
	.idnum			= kBMCT_firstUser,
	.use_camlink	= 1,
	.use_fiber		= 0,
	.fiber_mode		= 0,
	.burst_mode		= 0,
	.sizeX			= 8,
	.sizeY			= 8,
	.minLUT			= 1,				/* '1' is 1st actuator */
	.hva_lut		= sHVAF_lut,
	.active_map		= sHVAF_am,
	.description	= "HVAF:     Square    6x6,      CL,      (fake)"
	};
tE			err;
tBMCHVAsp	clone;

	MakeOpen();
	DDB(("DoAddF: open success\n"));
	/*
	**	We clone so that this can be freed like all other custom HVA spec
	*/
	if (NULL == (clone = CloneSpec(&sHVAF,sHVAF.idnum,sHVAF.description))) {
	  ERR(("DoAddf: clone failed!\n"));
	  return;
	  }
	DDB(("DoAddF: clone success\n"));

	if (kBMCEnoErr != (err = BMCaddCustomHVA(sBMC,clone))) {
	  ERR(("DoAddF: BMCaddCustomHVA gave %s\n",BMCgetErrStr(err)));
	  free(clone);
	  } else {
	  DB(("DoAddF: success\n"));
	  }

	return;
}
/*--------------------------------------------------------------------------*/
static void DoAddCustomHVA(int id, char *desc_str)
/*
**	Clone the current configuration and resubmit it with this id and
**	description string.
**
**	This is used to check the library HVA patterns.
*/
{
tBMC_HVAType_enum		mode;
tBMCHVAsp				sp,freeMe,clone;
tE						err;
int						sz;
	/*
	**	We need to do this to be sure we do not free() some static
	*/
	if (id < kBMCT_firstUser) {
	  ERR(("DoAddCustomHVA: cannot add id less than %d\n",kBMCT_firstUser));
	  return;
	  }

	MakeOpen();
	/*
	**	Get current status.
	*/
	if (kBMCEnoErr != (err = BMCgetCurrentHVAmode(sBMC,&mode))) {
	  ERR(("DoAddCustomHVA: BMCgetCurrentHVAmode gave '%s'\n",
			BMCgetErrStr(err)));
	  return;
	  }
	if (kBMCEnoErr != (err = BMCretrieveHVAinfo(sBMC,mode,&sp))) {
	  ERR(("DoAddCustomHVA: BMCretrieveHVAinfo gave '%s'\n",
			BMCgetErrStr(err)));
	  return;
	  }
	/*
	**	Clone the current spec
	*/
	if (NULL == (clone = CloneSpec(sp,id,desc_str))) {
	  ERR(("DoCustomHVA: clone failed!\n"));
	  return;
	  }
	/*
	**	See if the specified custom mode already exists -- if it does then
	**	we need to free it (because we put it there...).
	*/
	switch (err = BMCretrieveHVAinfo(sBMC,id,&freeMe)) {
	  default:
	  case kBMCErangeErr:
		ERR(("DoCustomHVA: slot %d returns '%s'\n",id,BMCgetErrStr(err)));
		free(clone);
		return;
	  case kBMCEnoErr:
		/*
		**	We got somthing -- it needs to be freed now.
		*/
		free(freeMe);
	  case kBMCEunknownErr:
		break;
	  }
	/*
	**	OK, if something was there it has been freed.  Send in the clone.
	*/
	if (kBMCEnoErr != (err = BMCaddCustomHVA(sBMC,clone))) {
	  ERR(("DoAddCustomHVA: BMCaddCustomHVA gave %s\n",BMCgetErrStr(err)));
	  free(clone);
	  } else {
	  DB(("DoAddCustomHVA: success\n"));
	  }

	return;
}
/*--------------------------------------------------------------------------*/
static void DumpAll(void)
/*
**	Show all available information from the board.
*/
{
tBMC_HVAType_enum		mode;
tBMC_orientation_enum	orien;
tBMCHVAsp				sp;
tBMCfiberStatusStruct	fs;
tBMCHVAccStruct			hs;
tBMCkiloStatusStruct	ks;
tE						err;
int						i,j,n,m;
char					fmtA[16],fmtB[16];
char					line[kBIGDATA];
tU32					data[kBIGDATA];
tU32					d2[kBIGDATA];
	/*
	**	Be sure we are open.
	*/
	MakeOpen();

	SHOW(("=== DumpAll: entry =======================\n"));

	if (kBMCEnoErr != (err = BMCgetCurrentHVAmode(sBMC,&mode))) {
	  ERR(("  BMCgetCurrentHVAmode gave '%s'\n",BMCgetErrStr(err)));
	  return;
	  }
	if (kBMCEnoErr != (err = BMCretrieveHVAinfo(sBMC,mode,&sp))) {
	  ERR(("  BMCretrieveHVAinfo gave '%s'\n",BMCgetErrStr(err)));
	  return;
	  }
	if (kBMCEnoErr != (err = BMCreadDMorientation(sBMC,&orien))) {
	  ERR(("  BMCreadDMorientation gave '%s'\n",BMCgetErrStr(err)));
	  return;
	  }
	SHOW(("  --- DumpAll: tHVAspec LUT is: ------------\n"));
	n = sp->sizeX * sp->sizeY;
	if (n < 10) {
	  m = 1;
	  } else if (n < 100) {
	  m = 2;
	  } else if (n < 1000) {
	  m = 3;
	  } else {
	  m = 4;
	  }
	(void)sprintf(fmtA,"%%0%0dd",m);
	(void)sprintf(fmtB,"%%%ds",m);
	DumpSpec(sp);
	/*
	**	Now show the LUT (reflects orientatin setting)
	*/
	if (kBMCEnoErr != (err = BMCreadHVALUT(sBMC,data))) {
	  ERR(("  BMCreadHVALUT gave '%s'\n",BMCgetErrStr(err)));
	  return;
	  }
	SHOW(("  --- DumpAll: actual LUT w/orien %d '%s': -\n",
			orien,BMCgetOrienStr(orien)));
	n = 0;
	for (i=0; i<sp->sizeY; i++) {
	  (void)sprintf(line,"  ");		/* new line */
	  for (j=0; j<sp->sizeX; j++) {
		if (0 != (data[n] & 0x010000)) {
		  (void)sprintf(line+strlen(line),fmtB,"");
		  (void)strcat(line,",");
		  } else {
		  (void)sprintf(line+strlen(line),fmtA,(data[n] & 0x0FFF));
		  (void)strcat(line,",");
		  }
		n += 1;						/* next entry */
		}
	  SHOW(("%s\n",line));
	  }
	if (kBMCEnoErr != (err = BMCreadHVAstate(sBMC,kBIGDATA,&n,d2))) {
	  ERR(("  BMCreadHVstate gave '%s'\n",BMCgetErrStr(err)));
	  return;
	  }
	SHOW(("  --- DumpAll: HVA state is %d (%d/%d) -----\n",
		n,sp->sizeY,sp->sizeX));
	n = 0;
	(void)strcpy(fmtA,"%04d");
	(void)strcpy(fmtB,"%4s");
	for (i=0; i<sp->sizeY; i++) {
	  (void)sprintf(line,"  ");		/* new line */
	  for (j=0; j<sp->sizeX; j++) {
		if (0 != (data[n] & 0x010000)) {
		  (void)sprintf(line+strlen(line),fmtB,"");
		  (void)strcat(line,",");
		  } else {
		  (void)sprintf(line+strlen(line),fmtA,d2[n]);
		  (void)strcat(line,",");
		  }
		n += 1;						/* next entry */
		}
	  SHOW(("%s\n",line));
	  }

	SHOW(("=== DumpAll: exit ========================\n"));

	return;
}
/*--------------------------------------------------------------------------*/
static tD GetTOD(void)
/*
**	Get fractional seconds since the epoch.
*/
{
struct timeval	tv;
tD				ans=0.0;

	(void)gettimeofday(&tv,NULL);
	ans = tv.tv_usec;
	ans /= 1000000.0;
	ans += tv.tv_sec;

	return(ans);
}
/*--------------------------------------------------------------------------*/
static void DoTestTime(tU32 nLoops)
/*
**	Exercise BMCburstHVA() in a loop and display results as transfers/sec.
*/
{
tBMC_HVAType_enum		mode;
tBMCHVAsp				sp;
tE						err;
tD						timeA,timeB;
int						neach=0,n,count=0,i;
tU16					data[kBIGDATA];

	MakeOpen();
	/*
	**	Get current status.
	*/
	if (kBMCEnoErr != (err = BMCgetCurrentHVAmode(sBMC,&mode))) {
	  ERR(("DoTestTime: BMCgetCurrentHVAmode gave '%s'\n",
			BMCgetErrStr(err)));
	  return;
	  }
	if (kBMCEnoErr != (err = BMCretrieveHVAinfo(sBMC,mode,&sp))) {
	  ERR(("DoTestTime: BMCretrieveHVAinfo gave '%s'\n",
			BMCgetErrStr(err)));
	  return;
	  }
	/*
	**	Zero the data so we do not stress hardware.
	*/
	(void)memset(data,'\0',sizeof(data));
	/*
	**	Count the active cells.
	*/
	n = sp->sizeX * sp->sizeY;
	for (i=0; i<n; i++) { neach += sp->active_map[i]; }

	timeA = GetTOD();
	for (i=0; i<nLoops; i++) {
	  if (kBMCEnoErr != (err = BMCburstHVA(sBMC,n,data))) {
		ERR(("DoTestTime: at %d of %d we got '%s'\n",
			i,nLoops,BMCgetErrStr(err)));
		break;
		}
	  count += neach;
	  }
	timeB = GetTOD();
	/*
	**	Compute data rate: we need to have run for at least 1mSec (which
	**	would NOT give a very good measurement, but at least we will not
	**	divide by zero).
	*/
	timeA = timeB - timeA;
	if (0.001 >= timeA) {
	  timeB = 0.0;
	  } else {
	  timeB = ((tD)(count))/timeA;
	  }

	SHOW(("DoTestTime: sent %d of %d (%d) in %.3lf sec -- %.3lf words/sec\n",
		nLoops,neach,count,timeA,timeB));

	return;
}
/*--------------------------------------------------------------------------*/
#define	kPI	3.14159265359

static void DoWriteSeq(tD delay)
/*
**	Generate best possible image rotation sequence(s)
*/
{
tBMCHVAsp				sp;
tBMC_HVAType_enum		mode;
tU16					seq[kBIGDATA];
tU8						mask[kBIGDATA];
int						n,m,i,j,k,iv;
tE						err;
tD						angle,sina,cosa,x0,y0,xv,yv,v;
	/*
	**	First we need to know how many full-mirror patterns will fit in the
	**	4k sequence storage.
	*/
	if (kBMCEnoErr != (err = BMCgetCurrentHVAmode(sBMC,&mode))) {
	  ERR(("DoWriteSeq: BMCgetCurrentHVAmode gave '%s'\n",BMCgetErrStr(err)));
	  return;
	  }
	if (kBMCEnoErr != (err = BMCretrieveHVAinfo(sBMC,mode,&sp))) {
	  ERR(("DoWriteSeq: BMCretrieveHVAinfo gave '%s'\n",BMCgetErrStr(err)));
	  return;
	  }

	n = kBMCmaxSeqLen / (sp->sizeX * sp->sizeY);
	if (n < 2) {
	  ERR(("DoWriteSeq: no point in a sequence of 1\n"));
	  return;
	  }
	/*
	**	So.  We can define n sequences in the available memory.  We are
	**	going to try for the best possible circle so
	**		n		sequenceAngles			pattern
	**		---		--------------			-------
	**		2		0/180					bar
	**		3		0/120/240				triangle
	**		4		0/90/180/270			diamond
	**		5		0/72/144/216/288		pentagon
	**		...
	*/
	x0 = ((tD)(sp->sizeX)) / 2.0;
	y0 = ((tD)(sp->sizeY)) / 2.0;
	m = 0;
	for (i=0; i<n; i++) {
	  angle = ((tD)(i)) * ((2.0 * kPI) / ((tD)(n)));
	  sina = sin(angle);
	  cosa = cos(angle);
	  for (j=0; j<sp->sizeY; j++) {
		yv = (((tD)(j)) - y0) * sina;
		for (k=0; k<sp->sizeX; k++) {
		  xv = (((tD)(k)) - x0) * cosa;
		  v = xv + yv;
		  iv = 0x08000 + ((int)((v/2.0) * 0x0FFFF));
		  if (iv < 0) { iv = 0; } else if (iv > 0x0FFFF) { iv = 0x0FFFF; }
		  seq[m++] = (tU16)iv;
		  }
		}
	  }
	/*
	**	Set the mask
	*/
	m = 0;
	for (j=0; j<sp->sizeY; j++) {
	  for (k=0; k<sp->sizeX; k++) {
		mask[m] = sp->active_map[m];
		m += 1;
		}
	  }
	/*
	**	All set.
	*/
	if (kBMCEnoErr != (err = BMCwriteSequence(
			sBMC,sp->sizeY,sp->sizeX,n,seq,mask,delay))) {
	  ERR(("DoWriteSeq: BMCwriteSequence gave %s\n",BMCgetErrStr(err)));
	  } else {
	  DB(("DoWriteSeq: BMCwriteSequence (%d patterns for %d/%d (%s)) OK\n",
			n,sp->sizeX,sp->sizeY,sp->description));
	  }

	return;
} 
/*--------------------------------------------------------------------------*/
#define	kSliceFraction		0.90

#define	kMaxDitherWave		65535

static void DoSetupDither(int cORC)
/*
**	Generate concave ('c') or convex ('C") dither pattern.
**
**	It is unrealistic to expect a hemisphere from a DM.
**
**	We will try for a slice off the top of a sphere at kSliceFraction of the
**	sphere radius.
*/
{
tBMCHVAsp				sp;
tBMC_HVAType_enum		mode;
tU16					gain[kBIGDATA];
tD						wave[kSMALLDATA];
int						i,j,c,m,iz;
tD						x0,y0,xv,yv,R,Rp,z;
tE						err;
	/*
	**	Concave or convex?
	*/
	switch (cORC) {
	  case 'c':
		c = 0;
		break;
	  case 'C':
		c = 1;
		break;
	  default:
		ERR(("DoSetupDither: '%c' is not 'c' or 'C'\n",cORC));
		return;
	  }
	/*
	**	Get the size info.
	*/
	if (kBMCEnoErr != (err = BMCgetCurrentHVAmode(sBMC,&mode))) {
	  ERR(("DoSetupDither: BMCgetCurrentHVAmode gave '%s'\n",
			BMCgetErrStr(err)));
	  return;
	  }
	if (kBMCEnoErr != (err = BMCretrieveHVAinfo(sBMC,mode,&sp))) {
	  ERR(("DoSetupDither: BMCretrieveHVAinfo gave '%s'\n",
			BMCgetErrStr(err)));
	  return;
	  }
	/*
	**	So the sphere is centered at [x0,y0,0]
	**		Our slice is at the plane z == kSliceFraction * r
	**
	**	At x0,y0 the actuator is at maximum for convex, minimum for concave
	**
	**	We generate the solultion for the convex case.
	*/
	x0 = (((tD)(sp->sizeX)) / 2.0) - 0.5;
	y0 = (((tD)(sp->sizeY)) / 2.0) - 0.5;
	/*
	**	Figure out R, the radius of the sphere
	**
	**	Start w/radious of circle on plane slicing sphere, Rprime
	*/
	xv = ((tD)(sp->sizeX)) - x0;
	yv = ((tD)(sp->sizeY)) - y0;
	Rp = sqrt((xv * xv) + (yv * yv));
	/*
	**	Now scale up to full sphere
	**
	**	R**2 = Rp**2 + (kSliceFraction*R)**2
	**	R**2 * (1 - kSliceFraction**2) = Rp**2
	**	R = sqrt((Rp**2)/(1-kSliceFraction**2))
	*/
	R = sqrt((Rp * Rp) / (1 - (kSliceFraction * kSliceFraction)));
	DB(("DoSetupDither: %d/%d gives R %.3lf, Rp %.3lf w/%.3lf slice\n",
		sp->sizeX,sp->sizeY,R,Rp,kSliceFraction));
	/*
	**	So max mirror height is equivalent to R*(1-kSliceFraction) and min
	**	mirror height is equivalent to R*kSliceFraction.  This scales to
	**	65535..0 for the dither values.
	**
	**	So z, the height of the mirror surface above the plane of the slice:
	**		At all 0==y: R**2 = x**2 + (z+(R*kSliceFraction))**2
	**		At all 0==x: R**2 = y**2 + (z+(R*kSliceFraction))**2
	**
	**		At x,y: R**2 = x**2 + y**2 + (z+(R*kSliceFraction))**2
	**
	**		z = sqrt(R**2 - (x**2 + y**2)) - R*kSliceFraction
	*/
	m = 0;
	for (i=0; i<sp->sizeY; i++) {
	  yv = ((tD)(i)) - y0;
	  for (j=0; j<sp->sizeX; j++) {
		xv = ((tD)(j)) - x0;
		z = sqrt(R*R - (xv*xv + yv*yv)) - (R * kSliceFraction);
		/*
		**	Now 0.0 <= z <= R*(1-kSliceFraction)
		*/
		z /= R * (1.0 - kSliceFraction);
		/*
		**	Now 0.0 <= z <= 1.0
		*/
		iz = (int)((z * (1.0 * kMaxDitherWave)) + 0.5);
		if (iz < 0) {
		  ERR(("DoSetupDither: %d/%d (%.1lf/%.1lf) gave %d/%.3lf, < 0\n",
				i,j,yv,xv,iz,z));
		  iz = 0;
		  } else if (iz > kMaxDitherWave) {
		  ERR(("DoSetupDither: %d/%d (%.1lf/%.1lf) gave %d/%.3lf, > %d\n",
				i,j,yv,xv,iz,z,kMaxDitherWave));
		  iz = kMaxDitherWave;
		  } else {
		  DDB(("DoSetupDither: %d/%d (%.1lf/%.1lf) gave %d/%.3lf, (ok)\n",
				i,j,yv,xv,iz,z));
		  }
		/*
		**	Deal w/concave vs. convex
		*/
		iz = (c * iz) + ((1-c)*(kMaxDitherWave-iz));
		/*
		**	And set the mirror value.
		*/
		gain[m++] = (tU16)(iz);
		}
	  }
	/*
	**	Now the triangle: values are 0 <= value <= 32767
	*/
	for (i=0; i<kSMALLDATA/2; i++) {
	  /*
	  **	Generate 0.0 <= value <= 1.0
	  */
	  z = ((tD)i) * (1.0 / ((tD)((kSMALLDATA/2)-1)));
	  /*
	  **	Triangle.
	  */
	  wave[i] = z;
	  wave[(kSMALLDATA-1)-i] = z;
	  }
	/*
	**	All set.
	*/
	if (kBMCEnoErr != (err = BMCwriteDitherGains(
			sBMC,sp->sizeX * sp->sizeY,gain))) {
	  ERR(("DoSetupDither: BMCwriteDitherGains gave %s\n",BMCgetErrStr(err)));
	  } else {
	  DB(("DoSetupDither: BMCwriteDitherGains OK\n"));
	  }
	if (kBMCEnoErr != (err = BMCwriteDitherWave(sBMC,kSMALLDATA,wave))) {
	  ERR(("DoSetupDither: BMCwriteDitherWave gave %s\n",BMCgetErrStr(err)));
	  } else {
	  DB(("DoSetupDither: BMCwriteDitherWave OK\n"));
	  }

	return;
} 
/*==========================================================================*/
static int GetPossibleHex(char *str)
/*
**	sscanf() and "%i" do not work as expected.
*/
{
int		ans=0,base=10,m1=1,x;
char	*estr=str;

	if (NULL == str) { return(0); }

	if (str[0] == '-') { m1 = -1; str += 1; }

	if ((str[0] == '0') && (str[1] == 'x')) { base = 16; str += 2; }

	while (*str != '\0') {
	  ans *= base;
	  x = 0;
	  if ((*str >= '0') && (*str <= '9')) { x = *str - '0'; }
		else if ((*str >= 'a') && (*str <= 'f')) { x += *str + 10 - 'a'; }
		else if ((*str >= 'A') && (*str <= 'F')) { x += *str + 10 - 'A'; }
		else { ERR(("GetPossibleHex: bad char '%c' of '%s'\n",*str,estr)); }
	  ans += x & 0x0FF;
	  str += 1;
	  }

	return(m1 * ans);
}
/*--------------------------------------------------------------------------*/
static void DecodeArgs(int argc, char **argv)
/*
**	Parse the input arguments.
*/
{
char	*str;
int		a1,a2,a3,a4,a5,i;
tE		err;
tU32	x;
tU32	data32[kBIGDATA];
tU16	data16[kBIGDATA];
tD		ad,add;

	argv += 1;	argc -= 1;					/* skip program name */

	while (argc-- > 0) {
	  DDB(("DecodeArgs: working on '%s'/%d\n",*argv,argc));
	  str = *argv++;
	  if (str[0] != '-') {
		ERR(("Do not know arg '%s'\n",str));
		ShowHelp();
		exit(1);
		}

	  switch (str[1]) {
		case 'h':	ShowHelp(); exit(0);
		case 'd':	sDebug += 1;									break;
		case 'l':
		  SHOW(("%s\n",*argv));
		  argv += 1; argc -= 1;
		  break;

		case 'b':	(void)sscanf(*argv++,"%d",&sNdx); argc -= 1;	break;
		case 'u':
		  (void)sscanf(*argv++,"%d",&a1); argc -= 1;
		  DB(("will sleep for %d usec\n",a1));
		  (void)usleep(a1);
		  break;

		case 'r':
		  (void)sscanf(*argv++,"%d",&a1); argc -= 1;
		  MakeOpen();
		  if (kBMCEnoErr != (err = BMCreadReg(sBMC,a1,&x))) {
			ERR(("DecodeArgs:BMCreadReg %p %d gave %s\n",
				sBMC,a1,BMCgetErrStr(err)));
			} else {
			SHOW(("BMCreadReg: %d gave 0x%08X/%d\n",a1,x,x));
			}
		  break;
		case 'R':
		  MakeOpen();
		  (void)sscanf(*argv++,"%d",&a1); argc -= 1;
		  (void)sscanf(*argv++,"%d",&a2); argc -= 1;
		  MakeOpen();
		  if (kBMCEnoErr != (err = BMCwriteReg(sBMC,a1,a2))) {
			ERR(("DecodeArgs:BMCwriteReg %p %d/%d gave %s\n",
				sBMC,a1,a2,BMCgetErrStr(err)));
			} else {
			DB(("BMCwriteReg: %d now 0x%08X/%d OK\n",a1,a2,a2));
			}
		  break;
		case 'B':
		  MakeOpen();
		  (void)sscanf(*argv++,"%d",&a1); argc -= 1;
		  (void)sscanf(*argv++,"%d",&a2); argc -= 1;
		  (void)sscanf(*argv++,"%d",&a3); argc -= 1;
		  (void)sscanf(*argv++,"%d",&a4); argc -= 1;
		  for (i=0; i<a1; i++) { data16[i] = (tU16)(a2 + (i * a3)); }
		  if (kBMCEnoErr != (err = BMCwriteBurst(sBMC,a2,a1,data16))) {
			ERR(("DecodeArgs:BMCwriteBurst %p %d %d gave %s\n",
				sBMC,a2,a1,BMCgetErrStr(err)));
			} else {
			DB(("BMCwriteBurst: %d/%d/%d/%d was OK\n",a1,a2,a3,a4));
			}
		  break;
		case 'a':
		  MakeOpen();
		  if (kBMCEnoErr != (err = BMCabortFrameData(sBMC))) {
			ERR(("DecodeArgs:BMCabortFrameData %p gave %s\n",
				sBMC,BMCgetErrStr(err)));
			} else {
			DB(("BMCabortFrameData: OK\n"));
			}
		  break;
		case 'D':
		  (void)sscanf(*argv++,"%d",&a1); argc -= 1;
		  MakeOpen();
		  if (kBMCEnoErr != (err = BMCsetSpinDelay(sBMC,a1))) {
			ERR(("DecodeArgs:BMCsetSpinDelay %p/%d gave %s\n",
				sBMC,a1,BMCgetErrStr(err)));
			} else {
			DB(("BMCsetSpinDelay: to %d OK\n",a1));
			}
		  break;

		case 'i':	DoInfo();										break;
		case 'F':	DoAddF();										break;
		case 'c':
		  (void)sscanf(*argv++,"%d",&a1); argc -= 1;
		  argc -= 1;
		  DoAddCustomHVA(a1,*argv++);
		  break;
		case 'I':
		  (void)sscanf(*argv++,"%d",&a1); argc -= 1;
		  MakeOpen();
		  if (kBMCEnoErr != (err = BMCsetUpHVA(sBMC,a1))) {
			ERR(("DecodeArgs:BMCsetUpHVA %p/%d gave %s\n",
				sBMC,a1,BMCgetErrStr(err)));
			} else {
			DB(("BMCsetUpHVA: to %d OK\n",a1));
			}
		  break;

		case 'O':
		  /*
		  **	NOTE: this internally tests BMCwriteHVALUT()
		  */
		  (void)sscanf(*argv++,"%d",&a1); argc -= 1;
		  (void)sscanf(*argv++,"%d",&a2); argc -= 1;
		  MakeOpen();
		  if (kBMCEnoErr != (err = BMCsetDMorientation(sBMC,a1,a2))) {
			ERR(("DecodeArgs:BMCsetDMorientation %p/%d/%d gave %s\n",
				sBMC,a1,a2,BMCgetErrStr(err)));
			} else {
			DB(("BMCsetDMorientation: to %d/%d OK\n",a1,a2));
			}
		  break;
		case 'p':
		  (void)sscanf(*argv++,"%d",&a1); argc -= 1;
		  (void)sscanf(*argv++,"%d",&a2); argc -= 1;
		  MakeOpen();
		  if (kBMCEnoErr != (err = BMCpokeDM(sBMC,((tU16)a1),((tU16)a2)))) {
			ERR(("DecodeArgs:BMCpokeDM %p/%d/%d gave %s\n",
				sBMC,a1,a2,BMCgetErrStr(err)));
			} else {
			DB(("BMCpokeDM: to %d/%d OK\n",a1,a2));
			}
		  break;
		case 'Z':
		  MakeOpen();
		  if (kBMCEnoErr != (err = BMCclearHVA(sBMC))) {
			ERR(("DecodeArgs:BMCclearHVA %p gave %s\n",
				sBMC,BMCgetErrStr(err)));
			} else {
			DB(("BMCclearHVA: OK\n"));
			}
		  break;
		case 'z':
		  (void)sscanf(*argv++,"%d",&a1); argc -= 1;
		  MakeOpen();
		  if (kBMCEnoErr != (err = BMCsetHVA(sBMC,((tU16)a1)))) {
			ERR(("DecodeArgs:BMCsetHVA %p/%d gave %s\n",
				sBMC,a1,BMCgetErrStr(err)));
			} else {
			DB(("BMCsetHVA: to %d OK\n",a1));
			}
		  break;

		case 'S':
		  (void)sscanf(*argv++,"%lf",&ad); argc -= 1;
		  MakeOpen();
		  if (kBMCEnoErr != (err = BMCenableSequence(sBMC,ad))) {
			ERR(("DecodeArgs:BMCenableSequence %p/%.3lf gave %s\n",
				sBMC,ad,BMCgetErrStr(err)));
			} else {
			DB(("BMCenableSequence: %.3lf OK\n",ad));
			}
		  break;
		case 's':
		  MakeOpen();
		  if (kBMCEnoErr != (err = BMCdisableSequence(sBMC))) {
			ERR(("DecodeArgs:BMCdisableSequence %p gave %s\n",
				sBMC,BMCgetErrStr(err)));
			} else {
			DB(("BMCdisableSequence: OK\n"));
			}
		  break;
		case 'W':
		  (void)sscanf(*argv++,"%lf",&ad); argc -= 1;
		  MakeOpen();
		  if (kBMCEnoErr != (err = BMCenableDither(sBMC,ad))) {
			ERR(("DecodeArgs:BMCenableDither %p/%.3lf gave %s\n",
				sBMC,ad,BMCgetErrStr(err)));
			} else {
			DB(("BMCenableDither: %.3lf OK\n",ad));
			}
		  break;
		case 'w':
		  MakeOpen();
		  if (kBMCEnoErr != (err = BMCdisableDither(sBMC))) {
			ERR(("DecodeArgs:BMCdisableDither %p gave %s\n",
				sBMC,BMCgetErrStr(err)));
			} else {
			DB(("BMCdisableDither: OK\n"));
			}
		  break;
		case 'j':
		  (void)sscanf(*argv++,"%lf",&ad); argc -= 1;
		  MakeOpen();
		  DoWriteSeq(ad);
		  break;
		case 'J':
		  a1 = (*argv)[0];	argv += 1; argc -= 1;
		  MakeOpen();
		  DoSetupDither(a1);
		  break;

		case 'T':
		  (void)sscanf(*argv++,"%d",&a1); argc -= 1;
		  (void)sscanf(*argv++,"%d",&a2); argc -= 1;
		  (void)sscanf(*argv++,"%d",&a3); argc -= 1;
		  (void)sscanf(*argv++,"%lf",&ad); argc -= 1;
		  MakeOpen();
		  if (kBMCEnoErr != (err = BMCtestTTLout(sBMC,a1,a2,a3,ad))) {
			ERR(("DecodeArgs:BMCtestTTLout %p/%d/%d/%d/%.6lf gave %s\n",
				sBMC,a1,a2,a3,ad,BMCgetErrStr(err)));
			} else {
			DB(("BMCtestTTLout: %d/%d/%d/%.6lf OK\n",a1,a2,a3,ad));
			}
		  break;

		case 'A':
		  MakeOpen();
		  DumpAll();
		  break;

		case 't':
		  (void)sscanf(*argv++,"%d",&a1); argc -= 1;
		  MakeOpen();
		  DoTestTime(a1);
		  break;

		default:
		  ERR(("Do not know arg '%s'\n",str));
		  ShowHelp();
		  exit(2);
		}
	  }

	return;
}
/*==========================================================================*/
int main(int argc, char **argv)
/*
**	Fetch the arguments and do what is requested
*/
{
	sArgv0 = *argv;

	DecodeArgs(argc,argv);

	if (NULL != sBMC) { (void)BMCclose(sBMC); }

	return(sExit);
}
/*==========================================================================*/

