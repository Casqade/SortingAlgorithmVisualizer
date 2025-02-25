#pragma once

#include <SortingAlgorithmVisualizer/fwd.hpp>

#include <cstdint>


///
/// \brief The RingBuffer class
///
/// Multiple producers, single consumer
///
class RingBuffer
{
  alignas(CacheLineSize) uint32_t mIndexReader {};

  alignas(CacheLineSize) uint32_t mIndexWriter {};
  alignas(CacheLineSize) uint32_t mIndexWritten {};


public:
  RingBuffer() = default;
  RingBuffer( const RingBuffer& ) = delete;
  RingBuffer( RingBuffer&& ) = delete;

  [[nodiscard]] uint32_t acquireWriteIndex();
  void commitWrittenIndex( uint32_t );

  uint32_t acquireReadIndex();
  bool canAcquireReadIndex();
};
