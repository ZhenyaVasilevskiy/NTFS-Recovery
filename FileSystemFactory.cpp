#include "FileSystemFactory.h"
#include "ntfs.h"


CBaseFileSystem	*CFileSystemFactory::GetFileSystem(const TCHAR *prmDisk)
{
	UCHAR		tmpBuf[512] = { 0 };
	
	UCHAR    ntfsSign[4] = { 0x4e, 0x54, 0x46, 0x53 };



	IBaseReader	*tmpReader = new CSectorReader;
	if (!tmpReader->OpenDevice(prmDisk))
	{
		delete tmpReader;
		return NULL;
	}
	if (tmpReader->ReadSector(0, 512, tmpBuf) != 512)
	{
		delete tmpReader;
		return NULL;
	}
	CBaseFileSystem	*fileSystem = NULL;
	if (memcmp(&tmpBuf[3], ntfsSign, 4) == 0)
	{
		fileSystem = new CNtfsFileSystem(tmpReader);
		fileSystem->SetStartSector(0);
		UINT64 *tmpTotalSectors = (UINT64*)(tmpBuf + 0x28);
		fileSystem->SetTotalSector(*tmpTotalSectors);
		fileSystem->Init();
	}
	return fileSystem;
}