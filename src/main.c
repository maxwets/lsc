#include <windows.h>
#include <winternl.h>
#include <stdio.h>

#include "helper.h"

INT
mainCRTStartup(VOID)
{
	DWORD  dwError      = FALSE;
	HANDLE hFile        = INVALID_HANDLE_VALUE;
	HANDLE hThread      = NULL;
	PVOID  pBaseAddress = NULL;
	DWORD  dwFileSize   = 0x00;
	DWORD  dwBytesRead  = 0x00;
	DWORD  dwOldProt    = 0x00;
	LPWSTR pwszCmdLine  = GetPeb()->ProcessParameters->CommandLine.Buffer;
	LPWSTR pwszFilePath = NULL;
	
#if !defined(_WIN64) && defined(_WIN32)
	pBaseAddress = (PVOID) 0x1400000;
#elif defined(_WIN64)
	pBaseAddress = (PVOID) 0x150000000;
#endif

	while (*pwszCmdLine && *pwszCmdLine != L' ') pwszCmdLine++;
	while (*pwszCmdLine == L' ') pwszCmdLine++;
	
	if (!*pwszCmdLine) {
		Print("Usage: program.exe <shellcode_path>\n");
		goto _end;
	}
	
	pwszFilePath = pwszCmdLine;

	hFile = API(KERNEL32, CreateFileW)(
		pwszFilePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE) {
		PrintError("Failed to open file", API(KERNEL32, GetLastError)());
		goto _end;
	}

	dwFileSize = API(KERNEL32, GetFileSize)(hFile, NULL);
	if (dwFileSize == INVALID_FILE_SIZE) {
		PrintError("Failed to get file size", API(KERNEL32, GetLastError)());
		goto _end;
	}

	pBaseAddress = API(KERNEL32, VirtualAlloc)(pBaseAddress, dwFileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (! pBaseAddress) {
		PrintError("Failed to allocate at target address", API(KERNEL32, GetLastError)());
		goto _end;
	}
	
	PrintHex(pBaseAddress);

	if (! API(KERNEL32, ReadFile)(hFile, pBaseAddress, dwFileSize, &dwBytesRead, NULL)) {
		PrintError("Failed to read file", API(KERNEL32, GetLastError)());
		goto _end;
	}

	Print("Press any key to execute\n");
	WaitForKey();

	if (! API(KERNEL32, VirtualProtect)(pBaseAddress, PAGE_ALIGN(dwFileSize), PAGE_EXECUTE_READ, &dwOldProt)) {
		PrintError("Failed to set memory protections", API(KERNEL32, GetLastError)());
		goto _end;
	}

	hThread = API(KERNEL32, CreateThread)(NULL, 0, (LPTHREAD_START_ROUTINE)pBaseAddress, NULL, 0, NULL);
	if (! hThread) {
		PrintError("Failed to create thread", API(KERNEL32, GetLastError)());
		goto _end;
	}

	API(KERNEL32, WaitForSingleObject)(hThread, INFINITE);
	API(KERNEL32, VirtualFree)(pBaseAddress, 0, MEM_RELEASE);

	dwError = ERROR_SUCCESS;
_end:
	if (hThread) API(KERNEL32, CloseHandle)(hThread);
	if (hFile != INVALID_HANDLE_VALUE) API(KERNEL32, CloseHandle)(hFile);
	Print("Execution finished. Press any key to exit.\n");
	WaitForKey();
	return dwError;
}
