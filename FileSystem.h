

#ifndef FILE_SYSTEM_INCLUDE_H
#define FILE_SYSTEM_INCLUDE_H

#include <windows.h>
#include <vector>
#include "File.h"
#include "reader.h"
#include "commutil.h"

using namespace std;

class CBaseFileSystem
{
public:
	
	CBaseFileSystem(IBaseReader	*prmReader=NULL);

	virtual ~CBaseFileSystem();
	
	virtual		UINT64	ReadBuf(UCHAR prmBuf[],UINT64 prmStartSector,UINT64 prmByteToRead);

	virtual		void	Init() = 0;

	virtual UINT64	ReadFileContent(CBaseFileObject *prmFileObject,
		UCHAR prmDstBuf[], UINT64 prmByteOff, UINT64 prmByteToRead) = 0;

	virtual void	GetDeletedFiles(vector<CBaseFileObject*> &fileArray) = 0;

	void		SetBytesPerSector(UINT16 prmBytesPerSector){m_bytesPerSector = prmBytesPerSector;}

	void		SetSectorsPerCluster(UINT8 prmSectorsPerCluster){m_sectorsPerCluster = prmSectorsPerCluster;}

	void		SetStartSector(UINT64 prmStartSector){m_startSector = prmStartSector;}

	void		SetTotalSector(UINT64 prmTotalSector){m_totalSector = prmTotalSector;}

protected:
	UINT16		m_bytesPerSector;
	UINT8		m_sectorsPerCluster;
	UINT64		m_startSector;
	UINT64		m_totalSector;
	IBaseReader	*m_reader;
};

#endif