#include <SortingAlgorithmVisualizer/Sorters/ISorter.hpp>
#include <SortingAlgorithmVisualizer/Allocators/IAllocator.hpp>

#include <windows.h>


ISorter::ISorter()
{
  InitializeCriticalSection(&mDataGuard);

  mRandomizeTask.dataGuard = &mDataGuard;
}

ISorter::~ISorter()
{
  DeleteCriticalSection(&mDataGuard);
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

BOOL
ISorter::tryLockData()
{
  return TryEnterCriticalSection(&mDataGuard);
}

void
ISorter::lockData()
{
  EnterCriticalSection(&mDataGuard);
}

void
ISorter::unlockData()
{
  LeaveCriticalSection(&mDataGuard);
}

RandomizeTask&
ISorter::getRandomizeTask()
{
  mRandomizeTask.callback = getRandomizeCallback();
  return mRandomizeTask;
}
