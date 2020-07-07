#include "reader.h"
#include "StringUtil.h"

using namespace commutil;

CSectorReader::CSectorReader()
{
	m_diskHandle = INVALID_HANDLE_VALUE;
}

CSectorReader::~CSectorReader()
{
	if(m_diskHandle != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_diskHandle);
	}
}

bool	CSectorReader::OpenDevice(const TCHAR *prmDevice)
{
	CStringUtil tmpStr;
	tmpStr.Format(TEXT("\\\\.\\%c:"), prmDevice[0]);
	m_diskHandle = ::CreateFile(tmpStr.GetString(), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE | FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_diskHandle == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	return true;
}

UINT64	CSectorReader::ReadSector(UINT64 prmStartSector, UINT64 prmBytesToRead, UCHAR *prmBuf)
{
	if (m_diskHandle == INVALID_HANDLE_VALUE)
	{
		return 0;
	}
	UINT32	tmpResult = 0;
	UINT32			tmpRemainBytes = prmBytesToRead % 512;
	UINT64			tmpBytesToRead = prmBytesToRead - tmpRemainBytes;
	LARGE_INTEGER	tmpInt;
	tmpInt.QuadPart = prmStartSector * 512;
	::SetFilePointerEx(m_diskHandle, tmpInt, NULL, FILE_BEGIN);
	DWORD	tmpBytesRead = 0;
	::ReadFile(m_diskHandle, prmBuf, (DWORD)tmpBytesToRead, &tmpBytesRead, NULL);
	tmpResult = tmpBytesRead;
	
	if (tmpRemainBytes != 0)
	{
		UCHAR	tmpBuf[512] = { 0 };
		::ReadFile(m_diskHandle, tmpBuf, 512, &tmpBytesRead, NULL);
		memcpy(prmBuf + tmpBytesToRead, tmpBuf, tmpRemainBytes);
		tmpResult += tmpRemainBytes;
	}
	return tmpResult;
}