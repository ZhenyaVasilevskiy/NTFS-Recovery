#include <stdio.h>
#ifdef _DEBUG
//#include <vld.h>
#endif
#include "FileSystemFactory.h"

void RestoreFile(CBaseFileSystem *prmFileSystem, CBaseFileObject *prmFileObject, string pathToFile)
{
	char	*tmpBuf = (char*)malloc(1024*1024);
	memset(tmpBuf, 0, 1024*1024);

	UINT64	tmpFileSize = prmFileObject->GetFileSize();
	UINT64	tmpBytesRead = 0;

	FILE	*fp = NULL;

	CStringUtil fileName = prmFileObject->GetFileName();
	string file = fileName.GetString();
	string path = pathToFile + file;
	const char* str = path.c_str();
	 
	fopen_s(&fp, str, "wb");

	while (tmpBytesRead < tmpFileSize)
	{
		std::cout << " .";

		UINT64  tmpVal = prmFileSystem->ReadFileContent(prmFileObject, (UCHAR*)tmpBuf, tmpBytesRead, 1024*1024);

		if (tmpVal == 0)
		{
			break;
		}

		tmpBytesRead += tmpVal;
		fwrite(tmpBuf, 1, (DWORD)tmpVal, fp);
	}

	free(tmpBuf);
	fclose(fp);
}

int main()
{
	CBaseFileSystem	*fileSystem = CFileSystemFactory::GetFileSystem(TEXT("C:\\Users\\User\Desktop\\red"));
	if (fileSystem == NULL)
	{
		return -1;
	}
	vector<CBaseFileObject *> fileArray;
	fileSystem->GetDeletedFiles(fileArray);
	if (fileArray.size() > 0)
	{
		int i = fileArray.size();
		while(i--) {
			RestoreFile(fileSystem, fileArray[i]);
		}
	}
	delete fileSystem;
	for (int i = 0; i < fileArray.size(); i++)
	{
		fileArray[i]->Destroy();
	}
	return 0;
}