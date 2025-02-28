#include <SortingAlgorithmVisualizer/MessageFormatting.hpp>

#include <cstdarg>


LPCSTR
FormatSystemMessage()
{
  return FormatSystemMessage( GetLastError() );
}

LPCSTR
FormatSystemMessage(
  DWORD errorCode )
{
  thread_local char messageBuf[256] {};

  auto result = FormatMessage(
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, errorCode, 0,
    messageBuf, sizeof(messageBuf),
    NULL );

  if ( result != 0 )
    return messageBuf;

  return nullptr;
}


LPCSTR
FormatUserMessage(
  LPCSTR message,
  va_list args )
{
  thread_local char messageBuf[256] {};

  auto result = FormatMessage(
    FORMAT_MESSAGE_FROM_STRING,
    message, 0, 0,
    messageBuf, sizeof(messageBuf),
    &args );

  if ( result != 0 )
    return messageBuf;

  return nullptr;
}

LPCSTR
FormatUserMessage(
  LPCSTR message,
  ... )
{
  va_list args {};
  va_start(args, message);

  auto result = FormatUserMessage(
    message, args );

  va_end(args);

  return result;
}

LPCSTR
FormatUserMessagePassthrough(
  LPCSTR message,
  ... )
{
  va_list args {};
  va_start(args, message);

  auto finalMessage = FormatUserMessage(
    message, args );

  va_end(args);

  if ( finalMessage != nullptr )
    return finalMessage;

  return message;
}
