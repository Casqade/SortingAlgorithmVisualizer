#pragma once

#include <intrin.h>

#include <utility>


template <typename T>
inline void
RandomizePlotDataRdtsc(
  void* data,
  size_t elementCount )
{
  const auto first = static_cast <T*> (data);

  for ( size_t i = elementCount - 1; i > 0; --i )
    std::swap(first[i], first[__rdtsc() % i]);
}
