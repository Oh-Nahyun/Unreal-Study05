#pragma once

#include <tchar.h>

void TRACE_WIN32(LPCTSTR lpszFormat, ...)
{
	TCHAR lpszBuffer[0x160];
	va_list fmtList;
	va_start(fmtList, lpszFormat);
	_vstprintf_s(lpszBuffer, lpszFormat, fmtList);
	va_end(fmtList);
	OutputDebugString(lpszBuffer);
}
