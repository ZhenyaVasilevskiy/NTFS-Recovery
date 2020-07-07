
#ifndef READER_INCLUDE_H
#define READER_INCLUDE_H
#include <windows.h>

class IBaseReader
{
public:
	virtual	~IBaseReader(){}
	
	virtual	bool	OpenDevice(const TCHAR *prmDevice) = 0;

	virtual	UINT64	ReadSector(UINT64 prmStartSector, UINT64 prmBytesToRead, UCHAR *prmBuf) = 0;
};

class CSectorReader:public IBaseReader
{
public:
	CSectorReader();

	~CSectorReader();
	
	virtual	bool OpenDevice(const TCHAR *prmDevice);

	virtual	UINT64	ReadSector(UINT64 prmStartSector, UINT64 prmBytesToRead, UCHAR *prmBuf);
private:
	HANDLE	m_diskHandle;
};

#endif