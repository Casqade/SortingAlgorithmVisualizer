#include <SortingAlgorithmVisualizer/Sorters/ISorter.hpp>
#include <SortingAlgorithmVisualizer/Allocators/IAllocator.hpp>
#include <SortingAlgorithmVisualizer/Atomics.hpp>

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


const auto SortingStepPresentTimeMs = 30;
const auto SortedPlotPresentTimeMs = 3000;

DWORD WINAPI
SorterThreadProc(
  LPVOID data )
{
  auto threadData = static_cast <ThreadLocalData*> (data);

  auto sorter = threadData->sorter;

  auto& sharedState = threadData->sharedState;

  bool isSorted {true};

  while ( AtomicLoadRelaxed(sharedState.shutdownRequested) == FALSE )
  {
    if ( isSorted == true )
    {
      Sleep(SortedPlotPresentTimeMs);


      auto& randomizerData = sharedState.randomizer;

      PVOID currentTask {};

      auto& newTask = sorter->getRandomizeTask();


      auto writeIndex =
        randomizerData.tasksIndices.acquireWriteIndex();

      auto taskIndex = writeIndex % randomizerData.tasks.size();

      randomizerData.tasks[taskIndex] = &newTask;

      randomizerData.tasksIndices.commitWrittenIndex(writeIndex);


      EnterCriticalSection(&randomizerData.tasksAvailableGuard);
      LeaveCriticalSection(&randomizerData.tasksAvailableGuard);
      WakeConditionVariable(&randomizerData.tasksAvailable);

//      TOOD: reset plot colors

      EnterCriticalSection(&newTask.taskFinishedGuard);

      while ( newTask.callback != nullptr )
      {
        auto resultCode = SleepConditionVariableCS(
          &newTask.taskFinished,
          &newTask.taskFinishedGuard,
          INFINITE );

          resultCode != 0;
      }

      LeaveCriticalSection(&newTask.taskFinishedGuard);

      sorter->reset();
    }
    else
      Sleep(SortingStepPresentTimeMs);

    isSorted = sorter->step();
  }

  return 0;
}
