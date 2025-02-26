#pragma once

#include <intrin.h>
#include <windows.h>

#include <algorithm>
#include <random>
#include <cstdlib>
#include <utility>


template <typename T>
inline void
RandomizePlotDataShuffle(
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

template <typename T>
inline void
RandomizePlotDataRand(
  void* data,
  size_t elementCount,
  CRITICAL_SECTION& dataGuard )
{
  const auto first = static_cast <T*> (data);

  EnterCriticalSection(&dataGuard);

  for ( size_t i = elementCount - 1; i > 0; --i )
    std::swap(first[i], first[rand() % i]);

  LeaveCriticalSection(&dataGuard);
}

template <typename T>
inline void
RandomizePlotDataRdtsc(
  void* data,
  size_t elementCount,
  CRITICAL_SECTION& dataGuard )
{
  const auto first = static_cast <T*> (data);

  EnterCriticalSection(&dataGuard);

  for ( size_t i = elementCount - 1; i > 0; --i )
    std::swap(first[i], first[__rdtsc() % i]);

  LeaveCriticalSection(&dataGuard);
}
