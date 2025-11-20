#include <windows.h>
#include <winternl.h>
#include <stdio.h>

#include "helper.h"


INLINE inline PPEB
GetPeb(VOID)
{
#if defined(_WIN64)
	return (PPEB)__readgsqword(0x60);
#else
	return (PPEB)__readfsdword(0x30);
#endif
}

NOINLINE VOID
Print(LPCSTR pszMessage)
{
	HANDLE hStdOut   = API(KERNEL32, GetStdHandle)(STD_OUTPUT_HANDLE);
	DWORD  dwLen     = 0x00;
	DWORD  dwWritten = 0x00;
	if (hStdOut == INVALID_HANDLE_VALUE) return;
	while (pszMessage[dwLen]) dwLen++;
	API(KERNEL32, WriteConsoleA)(hStdOut, pszMessage, dwLen, &dwWritten, NULL);
}

NOINLINE VOID
PrintHex(PVOID pAddress)
{
	CHAR     szBuffer[32] = { 0x00 };
	DWORD    i            = 0x00;
	ULONG_PTR ullValue    = (ULONG_PTR) pAddress;
	szBuffer[i++] = '0';
	szBuffer[i++] = 'x';
	for (INT j = sizeof(ULONG_PTR) * 2 - 1; j >= 0x00; j--) {
		BYTE bNibble = (ullValue >> (j * 4)) & 0x0F;
		szBuffer[i++] = bNibble < 0x0A ? '0' + bNibble : 'a' + bNibble - 0x0A;
	}
	szBuffer[i++] = '\n';
	szBuffer[i] = '\0';
	Print(szBuffer);
}

NOINLINE VOID
PrintError(LPCSTR pszMessage, DWORD dwError)
{
	CHAR szBuffer[64] = { 0x00 };
	DWORD i	          = 0x00;
	DWORD j	          = 0x00;
	while (pszMessage[j]) szBuffer[i++] = pszMessage[j++];
	szBuffer[i++] = ':';
	szBuffer[i++] = ' ';
	szBuffer[i++] = '0';
	szBuffer[i++] = 'x';
	for (INT k = 7; k >= 0; k--) {
		BYTE bNibble = (dwError >> (k * 4)) & 0xF;
		szBuffer[i++] = bNibble < 0x0A ? '0' + bNibble : 'a' + bNibble - 0x0A;
	}
	szBuffer[i++] = '\n';
	szBuffer[i] = '\0';
	Print(szBuffer);
}

NOINLINE VOID
WaitForKey(VOID)
{
	HANDLE hStdIn  = API(KERNEL32, GetStdHandle)(STD_INPUT_HANDLE);
	CHAR   cBuffer = 0x00;
	DWORD  dwRead  = 0x00;
	if (hStdIn != INVALID_HANDLE_VALUE) {
		API(KERNEL32, ReadConsoleA)(hStdIn, &cBuffer, 1, &dwRead, NULL);
	}
}
