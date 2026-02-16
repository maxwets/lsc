#pragma once

#ifndef _LSC_HELPER_H
#define _LSC_HELPER_H

#define API(m, f)     (f)
#define SIZE_OF_PAGE  (0x1000)
#define PAGE_ALIGN(x) (((ULONG_PTR)x) + ((SIZE_OF_PAGE - (((ULONG_PTR)x) & (SIZE_OF_PAGE - 1))) % SIZE_OF_PAGE))
#define INLINE        __attribute__((__always_inline__))
#define NOINLINE      __attribute__((__noinline__))

PPEB        GetPeb(VOID);
VOID        Print(_In_ LPCSTR pszMessage);
VOID        PrintF(_In_ LPCSTR szFormat, ...);
VOID        PrintError(_In_ LPCSTR pszMessage, _In_ DWORD dwError);
PCHAR       GetExceptionName(_In_ DWORD dwExceptionCode);
VOID        WaitForKey(VOID);
LONG WINAPI ExceptionHandler(_In_ PEXCEPTION_POINTERS pExceptionInfo);

#endif // _LSC_HELPER_H
