#include <SortingAlgorithmVisualizer/Sorters/ISorter.hpp>
#include <SortingAlgorithmVisualizer/Allocators/IAllocator.hpp>
#include <SortingAlgorithmVisualizer/Atomics.hpp>

#include <windows.h>

#include <cassert>
#include <cstring>


ISorter::ISorter()
{
  InitializeCriticalSection(&mRandomizeTask.taskFinishedGuard);
  InitializeConditionVariable(&mRandomizeTask.taskFinished);
}

ISorter::~ISorter()
{
  mColors.deinit();

  DeleteCriticalSection(&mDataGuard);
  DeleteCriticalSection(&mRandomizeTask.taskFinishedGuard);
}

void
ISorter::Destroy(
  ISorter* sorter )
{
  auto allocator = ReadObjectAddress <IAllocator> (
    reinterpret_cast <uintptr_t> (sorter) + sorter->instanceSize() );

  sorter->~ISorter();

  allocator->deallocate(sorter);
}

bool
ISorter::init(
  size_t valueCount,
  IAllocator& allocator )
{
  if ( mColors.init(valueCount, allocator) == false )
  {
    MessageBox( NULL,
      "Failed to initialize plot colors: "
      "Out of memory budget",
      NULL, MB_ICONERROR );

    ProgramShouldAbort = true;
    return false;
  }

  return true;
}

void
ISorter::swapBuffers()
{
  mBackBufferIndex = InterlockedExchangeNoFence(
    &mFrontBufferIndex,
    mBackBufferIndex );

  AtomicStoreRelaxed(mBufferWasSwapped, 1);
}

LONG
ISorter::acquireBuffer(
  LONG releasedBufferIndex )
{
  if ( InterlockedExchangeNoFence(&mBufferWasSwapped, 0) == 0 )
    return releasedBufferIndex;

  return InterlockedExchangeAcquire(
    &mFrontBufferIndex,
    releasedBufferIndex );
}

void
ISorter::mapValuesBuffer(
  void* gpuBuffer )
{
  assert(gpuBuffer != nullptr);
  assert(mValuesBuffer == nullptr);

  mValuesBuffer = gpuBuffer;
}

void
ISorter::mapColorsBuffer(
  void* gpuBuffer )
{
  assert(gpuBuffer != nullptr);
  assert(mColorsBuffer == nullptr);

  mColorsBuffer = gpuBuffer;
}

void
ISorter::flushValues() const
{
  auto valuesBufferSize =
    valueSize() * valueCount();

  auto valuesBufferStart =
    reinterpret_cast <uintptr_t> (mValuesBuffer)
    + mBackBufferIndex * valuesBufferSize;

  std::memcpy(
    reinterpret_cast <void*> (valuesBufferStart),
    mRandomizeTask.data,
    valuesBufferSize );
}

void
ISorter::flushColors() const
{
  using PlotValueColorIndex::PlotValueColorIndex;

  auto colorsBufferSize =
    sizeof(PlotValueColorIndex) * valueCount();

  auto colorsBufferStart =
    reinterpret_cast <uintptr_t> (mColorsBuffer)
    + mBackBufferIndex * colorsBufferSize;

  std::memcpy(
    reinterpret_cast <void*> (colorsBufferStart),
    mColors.data(),
    colorsBufferSize );
}

void
ISorter::resetColors()
{
  std::memset(
    mColors.data(),
    PlotValueColorIndex::Unsorted,
    mColors.size() );
}

size_t
ISorter::valueCount() const
{
  return mColors.size();
}

RandomizeTask&
ISorter::getRandomizeTask()
{
  mRandomizeTask.callback = getRandomizeCallback();
  return mRandomizeTask;
}

size_t
ISorter::HeapMemoryBudget(
  size_t valueCount )
{
  using PlotValueColorIndex::PlotValueColorIndex;

  return sizeof(PlotValueColorIndex) * valueCount;
}
