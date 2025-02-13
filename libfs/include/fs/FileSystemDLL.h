#pragma once

#define ImportFS   __declspec( dllimport )
#define ExportFS   __declspec( dllexport )


#ifndef FILESYSTEM_EXPORTS
#define FSExport ImportFS
#else
#define FSExport ExportFS
#endif


#ifdef _DEBUG 
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
bool static _trace(CHAR *format, ...)
{
	CHAR buffer[1000]; 
	va_list argptr; 
	va_start(argptr, format); 
	vsprintf_s(buffer, format, argptr); 
	va_end(argptr); 
	OutputDebugStringA(buffer); 
	return true; 
}
#define DEBUG_SHOW _trace 
#else 
#define DEBUG_SHOW false /*&& _trace */
#endif