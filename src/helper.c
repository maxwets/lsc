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
Print(_In_ LPCSTR pszMessage)
{
	HANDLE hStdOut   = API(KERNEL32, GetStdHandle)(STD_OUTPUT_HANDLE);
	DWORD  dwLen	 = 0x00;
	DWORD  dwWritten = 0x00;
	if (hStdOut == INVALID_HANDLE_VALUE) return;
	while (pszMessage[dwLen]) dwLen++;
	API(KERNEL32, WriteConsoleA)(hStdOut, pszMessage, dwLen, &dwWritten, NULL);
}

NOINLINE VOID
PrintF(_In_ LPCSTR pszFormat, ...)
{
	HANDLE hStdOut = API(KERNEL32, GetStdHandle)(STD_OUTPUT_HANDLE);
	CHAR   szBuffer[0x1000];
	DWORD  dwWritten = 0x00;
	INT	iLen = 0x00;
	va_list args;
	if (hStdOut == INVALID_HANDLE_VALUE)
		return;
	va_start(args, pszFormat);
	iLen = API(NTDLL, _vsnprintf)(szBuffer, sizeof(szBuffer) - 1, pszFormat, args);
	va_end(args);
	if (iLen < 0 || iLen >= sizeof(szBuffer))
		return;
	szBuffer[iLen] = '\0';
	API(KERNEL32, WriteConsoleA)(hStdOut, szBuffer, iLen, &dwWritten, NULL);
}

NOINLINE VOID
PrintError(_In_ LPCSTR pszMessage, _In_ DWORD dwError)
{
	CHAR szBuffer[512] = { 0x00 };
	CHAR szErrorMsg[256] = { 0x00 };
	DWORD i	= 0x00;
	DWORD j	= 0x00;

	while (pszMessage[j]) szBuffer[i++] = pszMessage[j++];
	szBuffer[i++] = ':';
	szBuffer[i++] = ' ';
	szBuffer[i++] = '0';
	szBuffer[i++] = 'x';

	for (INT k = 7; k >= 0; k--) {
		BYTE bNibble = (dwError >> (k * 4)) & 0xF;
		szBuffer[i++] = bNibble < 0x0A ? '0' + bNibble : 'a' + bNibble - 0x0A;
	}

	DWORD dwLen = API(KERNEL32, FormatMessageA)(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		szErrorMsg,
		sizeof(szErrorMsg),
		NULL
	);

	if (dwLen > 0) {
		while (dwLen > 0 && (szErrorMsg[dwLen - 1] == '\n' || szErrorMsg[dwLen - 1] == '\r')) {
			szErrorMsg[--dwLen] = '\0';
		}

		szBuffer[i++] = ' ';
		szBuffer[i++] = ':';
		szBuffer[i++] = ' ';
		j = 0;
		while (szErrorMsg[j] && i < sizeof(szBuffer) - 3) {
			szBuffer[i++] = szErrorMsg[j++];
		}
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

NOINLINE PCHAR
GetExceptionName(_In_ DWORD dwExceptionCode)
{
	switch (dwExceptionCode) {
		case 0xC0000005: return "ACCESS_VIOLATION";
		case 0xC0000094: return "INTEGER_DIVIDE_BY_ZERO";
		case 0xC000008C: return "ARRAY_BOUNDS_EXCEEDED";
		case 0xC000008D: return "FLOAT_DENORMAL_OPERAND";
		case 0xC000008E: return "FLOAT_DIVIDE_BY_ZERO";
		case 0xC000008F: return "FLOAT_INEXACT_RESULT";
		case 0xC0000090: return "FLOAT_INVALID_OPERATION";
		case 0xC0000091: return "FLOAT_OVERFLOW";
		case 0xC0000092: return "FLOAT_STACK_CHECK";
		case 0xC0000093: return "FLOAT_UNDERFLOW";
		case 0xC0000095: return "INTEGER_OVERFLOW";
		case 0xC0000096: return "PRIVILEGED_INSTRUCTION";
		case 0xC00000FD: return "STACK_OVERFLOW";
		case 0xC000001D: return "ILLEGAL_INSTRUCTION";
		case 0x80000001: return "GUARD_PAGE";
		case 0x80000003: return "BREAKPOINT";
		case 0x80000004: return "SINGLE_STEP";
		case 0xC000013A: return "CTRL_C_EXIT";
		case 0xC0000409: return "STACK_BUFFER_OVERRUN";
		case 0xC0000417: return "INVALID_CRUNTIME_PARAMETER";
		case 0x406D1388: return "MS_VC_EXCEPTION";
		default: return "UNKNOWN";
	}
}

NOINLINE LONG WINAPI
ExceptionHandler(_In_ PEXCEPTION_POINTERS pExceptionInfo)
{
	PCONTEXT          pCtx    = pExceptionInfo->ContextRecord;
	PEXCEPTION_RECORD pExcept = pExceptionInfo->ExceptionRecord;

	PrintF("\n[!] Exception: 0x%08X (%s) @ 0x%p\n", pExcept->ExceptionCode, GetExceptionName(pExcept->ExceptionCode), pExcept->ExceptionAddress);

#if defined(_M_X64)
	PrintF("\n[*] Registers:\n");
	PrintF("RAX:\t%016llX\nRBX:\t%016llX\nRCX:\t%016llX\nRDX:\t%016llX\n"
			"RSI:\t%016llX\nRDI:\t%016llX\nRBP:\t%016llX\nRSP:\t%016llX\n"
			"R8:\t%016llX\nR9:\t%016llX\nR10:\t%016llX\nR11:\t%016llX\n"
			"R12:\t%016llX\nR13:\t%016llX\nR14:\t%016llX\nR15:\t%016llX\n"
			"RIP:\t%016llX\nEFLAGS:\t%08X\n",
			pCtx->Rax, pCtx->Rbx, pCtx->Rcx, pCtx->Rdx,
			pCtx->Rsi, pCtx->Rdi, pCtx->Rbp, pCtx->Rsp,
			pCtx->R8, pCtx->R9, pCtx->R10, pCtx->R11,
			pCtx->R12, pCtx->R13, pCtx->R14, pCtx->R15,
			pCtx->Rip, pCtx->EFlags);

	PrintF("\n[*] Stack:\n");
	PULONG64 pqwStack = (PULONG64)pCtx->Rsp;
	for (int i = 0x00; i < 0x40; i++) {
		PrintF("RSP+%03X (%016llX):\t%016llX\n", (i * 8), (ULONG64)&pqwStack[i], pqwStack[i]);
	}
#elif defined(_M_IX86)
	PrintF("\n[*] Registers:\n");
	PrintF("EAX:\t%08X\nEBX:\t%08X\nECX:\t%08X\nEDX:\t%08X\n"
			"ESI:\t%08X\nEDI:\t%08X\nEBP:\t%08X\nESP:\t%08X\n"
			"EIP:\t%08X\nEFLAGS:\t%08X\n",
		   pCtx->Eax, pCtx->Ebx, pCtx->Ecx, pCtx->Edx,
		   pCtx->Esi, pCtx->Edi, pCtx->Ebp, pCtx->Esp,
		   pCtx->Eip, pCtx->EFlags);

	PrintF("\n[*] Stack:\n");
	PULONG pdwStack = (PULONG)pCtx->Esp;
	for (int i = 0x00; i < 0x40; i++) {
		PrintF("ESP+%02X (%08X):\t%08X\n", (i * 4), (ULONG)&pdwStack[i], pdwStack[i]);
	}
#endif // defined(_M_X64) || defined(_M_IX86)
	return EXCEPTION_CONTINUE_SEARCH;
}
