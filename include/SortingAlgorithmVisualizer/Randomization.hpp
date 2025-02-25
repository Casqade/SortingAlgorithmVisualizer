#pragma once

#include <windows.h>

#include <algorithm>
#include <random>


template <typename T>
inline void
RandomizePlotData(
  void* data,
  size_t elementCount,
  CRITICAL_SECTION& dataGuard )
{
  thread_local std::random_device rd;
  thread_local std::minstd_rand0 g(rd());

  const auto first = static_cast <T*> (data);
  const auto last = first + elementCount;


  EnterCriticalSection(&dataGuard);

  std::shuffle(first, last, g);

  LeaveCriticalSection(&dataGuard);
}


