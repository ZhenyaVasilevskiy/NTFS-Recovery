
#ifndef FILE_SYSTEM_FACTORY_INCLUDE_H
#define FILE_SYSTEM_FACTORY_INCLUDE_H
#include "FileSystem.h"

class CFileSystemFactory
{
public:
	static CBaseFileSystem	*GetFileSystem(const TCHAR *prmDisk);
};

#endif