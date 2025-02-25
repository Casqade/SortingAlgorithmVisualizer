#include <SortingAlgorithmVisualizer/Containers/RingBuffer.hpp>
#include <SortingAlgorithmVisualizer/Atomics.hpp>


uint32_t
RingBuffer::acquireWriteIndex()
{
  return AtomicIncrementRelaxed(mIndexWriter) - 1;
}

void
RingBuffer::commitWrittenIndex(
  uint32_t bufferIndex )
{
  decltype(mIndexWritten) indexWritten;

  do
  {
    indexWritten = AtomicCompareExchangeRelease(
      mIndexWritten, bufferIndex, bufferIndex + 1 );

  } while ( indexWritten != bufferIndex );
}

uint32_t
RingBuffer::acquireReadIndex()
{
  return mIndexReader++;
}

bool
RingBuffer::canAcquireReadIndex()
{
  return mIndexReader != AtomicLoadAcquire(mIndexWritten);
}
