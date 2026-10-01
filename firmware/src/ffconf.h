/*---------------------------------------------------------------------------/
/  Configuration of FatFs module for MotoRecoPico
/---------------------------------------------------------------------------*/

#define FFCONF_DEF	80386	/* Revision ID (FatFs R0.15) */

/*---------------------------------------------------------------------------/
/ Function Configurations
/---------------------------------------------------------------------------*/

#define FF_FS_READONLY	0
#define FF_FS_MINIMIZE	0

/* f_rename is needed: a trip starts under a placeholder name and is renamed both when GPS
/  finally reports the date and again when the ignition goes off (see logger.c) */
#define FF_USE_FIND		0
#define FF_USE_MKFS		0	/* cards are formatted on a PC. never reformat one in the field */
#define FF_USE_FASTSEEK	0
#define FF_USE_EXPAND	0

/* f_chmod is needed to give the settings file the hidden attribute (settings_file.c) */
#define FF_USE_CHMOD	1
#define FF_USE_LABEL	0
#define FF_USE_FORWARD	0

/* f_gets reads the settings file a line at a time. 1 is the plain byte oriented flavour */
#define FF_USE_STRFUNC	1
#define FF_PRINT_LLI	0
#define FF_PRINT_FLOAT	0
#define FF_STRF_ENCODE	0

/*---------------------------------------------------------------------------/
/ Locale and Namespace Configurations
/---------------------------------------------------------------------------*/

#define FF_CODE_PAGE	437

/* long file names are not optional here: "20260731_120423.dat" is 19 characters and does not fit
/  the 8.3 short name form the raspberry pi version's file names have to match */
#define FF_USE_LFN		1	/* 1: enabled, LFN working buffer on the stack */
#define FF_MAX_LFN		64
#define FF_LFN_UNICODE	0	/* ANSI/OEM. all file names this firmware creates are ASCII */
#define FF_LFN_BUF		64
#define FF_SFN_BUF		12
#define FF_FS_RPATH		0	/* every path this firmware uses is absolute from the root */
#define FF_PATH_DEPTH	10

/*---------------------------------------------------------------------------/
/ Drive/Volume Configurations
/---------------------------------------------------------------------------*/

#define FF_VOLUMES		1
#define FF_STR_VOLUME_ID	0
#define FF_VOLUME_STRS		"RAM","NAND","CF","SD","SD2","USB","USB2","USB3"
#define FF_MULTI_PARTITION	0
#define FF_MIN_SS		512
#define FF_MAX_SS		512
#define FF_LBA64		0
#define FF_MIN_GPT		0x10000000
#define FF_USE_TRIM		0

/*---------------------------------------------------------------------------/
/ System Configurations
/---------------------------------------------------------------------------*/

/* FF_FS_TINY 0 keeps a 512 byte sector buffer per open file. that costs a kilobyte for the trip
/  log plus the debug log and saves a re-read of the sector on every write, which is the hot path */
#define FF_FS_TINY		0

#define FF_FS_EXFAT		0	/* FAT32 only. exFAT needs a licence notice and buys nothing here */

/* 0 means FatFs calls get_fattime(). the timestamps come from GPS, so files carry a real date
/  once the receiver has a fix (diskio.c) */
#define FF_FS_NORTC		0
#define FF_NORTC_MON	1
#define FF_NORTC_MDAY	1
#define FF_NORTC_YEAR	2026
#define FF_FS_CRTIME	0	/* creation time is not tracked. the file name carries the time */

#define FF_FS_NOFSINFO	0
#define FF_FS_LOCK		0
#define FF_FS_REENTRANT	0	/* single threaded: everything runs in the main loop */
#define FF_FS_TIMEOUT	1000
