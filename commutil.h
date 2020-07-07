

#ifndef COMM_UTIL_INCLUDE_H
#define COMM_UTIL_INCLUDE_H
#include <windows.h>
#include<stdlib.h>

#include "StringUtil.h"

using namespace commutil;

void	rev16bit(UCHAR *value);

void	rev32bit(UCHAR *value);

void	rev64bit(UCHAR *value);

#pragma pack(push,1)

typedef struct _File_Content_Extent
{
	UINT64	startSector;
	UINT64	totalSector;
	UINT8	isPersist;

	_File_Content_Extent *next;
	_File_Content_Extent()
	{
		isPersist = 0;
		startSector = 0;
		totalSector = 0;
		next = NULL;
	}
}File_Content_Extent_s;
#pragma pack(pop)

#pragma pack(push,1)


typedef struct dir_entry {
	unsigned char   name[11];/* name and extension */
	unsigned char   attr;           /* attribute bits */
	unsigned char   lcase;          /* Case for base and extension */
	unsigned char   ctime_cs;       /* Creation time, centiseconds (0-199) */
	unsigned short  ctime;          /* Creation time */
	unsigned short  cdate;          /* Creation date */
	unsigned short  adate;          /* Last access date */
	unsigned short  starthi;        /* High 16 bits of cluster in FAT32 */
	unsigned short  time, date, start;/* time, date and first cluster */
	unsigned int    size;           /* file size (in bytes) */
}DIR_ENTRY_s;


#define EOC 0x0FFFFFFF

typedef struct dir_long_entry {
	unsigned char    id;             /* sequence number for slot */
	unsigned char    name0_4[10];    /* first 5 characters in name */
	unsigned char    attr;           /* attribute byte */
	unsigned char    reserved;       /* always 0 */
	unsigned char    alias_checksum; /* checksum for 8.3 alias */
	unsigned char    name5_10[12];   /* 6 more characters in name */
	unsigned short   start;         /* starting cluster number, 0 in long slots */
	unsigned char    name11_12[4];   /* last 2 characters in name */
}DIR_LONG_ENTRY_s;

#pragma pack(pop)

#pragma pack(push,1)

#define MFTREFMASK	0xFFFFFFFFFFFF


enum NTFS_ATTRDEF
{
	ATTR_STANDARD = 0x10,
	ATTR_ATTRIBUTE_LIST = 0x20,
	ATTR_FILE_NAME = 0x30,
	ATTR_VOLUME_VERSION = 0x40,
	ATTR_SECURITY_DESCRIPTOR = 0x50,
	ATTR_VOLUME_NAME = 0x60,
	ATTR_VOLUME_INFOMATION = 0x70,
	ATTR_DATA = 0x80,
	ATTR_INDEX_ROOT = 0x90,
	ATTR_INDEX_ALLOCATION = 0xA0,
	ATTR_BITMAP = 0xB0,
	ATTR_SYMLINK = 0xC0,
	ATTR_HPFS_EXTENDED_INFO = 0xD0,
	ATTR_HPFS_EXTENDED = 0xE0,
	ATTR_PROPERTY = 0xF0,
	ATTR_LOG_STREAM = 0x100
};

typedef struct _NTFS_Data_Run
{
	UINT64				lcn;   
	UINT64				vcn;	
	UINT64				length;	
	_NTFS_Data_Run		*next;
	_NTFS_Data_Run()
	{
		lcn = 0;
		vcn = 0;
		length = 0;
		next = NULL;
	}
}Ntfs_Data_Run;
#pragma pack(pop)

#endif