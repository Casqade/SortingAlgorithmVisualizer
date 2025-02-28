#pragma once

#include <windows.h>

#include <cstdarg>


LPCSTR FormatSystemMessage();
LPCSTR FormatSystemMessage( DWORD errorCode );

LPCSTR FormatUserMessage( LPCSTR message, ... );
LPCSTR FormatUserMessage( LPCSTR message, va_list args );

LPCSTR FormatUserMessagePassthrough( LPCSTR message, ... );
