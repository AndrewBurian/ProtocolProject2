#include "BCP.h"
#include <queue>

//take 'file name', read file to queue one byte at a time.
DWORD WINAPI FileBufferThread(LPVOID threadParams)
{
	LPCSTR fileName = ((SHARED_DATA_POINTERS*)threadParams)->p_outFileName;
	queue<BYTE> *outQueue=((SHARED_DATA_POINTERS*)threadParams)-> p_quOutputQueue;
	BYTE   bySwap ;
	DWORD  dwBytesRead ;
	HANDLE hFile ;
	int    i, iFileLength, iUniTest ;
	PBYTE  pBuffer;

	if (INVALID_HANDLE_VALUE == 
		(hFile = CreateFile ((LPCWSTR)fileName, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, 0, NULL)))
		return FALSE ;

	iFileLength = GetFileSize (hFile, NULL) ; 
	pBuffer= (PBYTE)malloc (iFileLength + 2) ;
	ReadFile (hFile, pBuffer, iFileLength, &dwBytesRead, NULL) ;
	CloseHandle (hFile) ;
	pBuffer[iFileLength] = '\0' ;
	pBuffer[iFileLength + 1] = '\0' ;
	for (int i=0;i<iFileLength;++i){
		outQueue->push(pBuffer[i]);
	}	
	free (pBuffer) ;

	return TRUE ;
}
