#include <SortingAlgorithmVisualizer/Sorters/ISorter.hpp>
#include <SortingAlgorithmVisualizer/Allocators/IAllocator.hpp>

#include <windows.h>


ISorter::ISorter()
{
  InitializeCriticalSection(&mDataGuard);
  InitializeCriticalSection(&mRandomizeTask.taskFinishedGuard);
  InitializeConditionVariable(&mRandomizeTask.taskFinished);

  mRandomizeTask.dataGuard = &mDataGuard;
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

size_t
ISorter::HeapMemoryBudget(
  size_t valueCount )
{
  return sizeof(PlotValueColorIndex) * valueCount;
}
