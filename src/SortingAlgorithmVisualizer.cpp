#include <SortingAlgorithmVisualizer/Atomics.hpp>
#include <SortingAlgorithmVisualizer/Containers.hpp>
#include <SortingAlgorithmVisualizer/CommonTypes.hpp>
#include <SortingAlgorithmVisualizer/Sorters/MockSorter.hpp>
#include <SortingAlgorithmVisualizer/Allocators/ArenaAllocator.hpp>



const auto SortingStepPresentTime =
  30;

const auto SortedResultPresentTime =
  3000;


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
      Sleep(SortedResultPresentTime);


      auto& randomizerData = sharedState.randomizer;

      PVOID currentTask {};

      auto& newTask = sorter->getRandomizeTask();

      do
      {
        currentTask = AtomicPointerCompareExchangeRelease(
          randomizerData.task, nullptr, &newTask );

      } while ( currentTask != nullptr );


      EnterCriticalSection(&randomizerData.taskAvailableGuard);
      LeaveCriticalSection(&randomizerData.taskAvailableGuard);
      WakeConditionVariable(&randomizerData.taskAvailable);

//      TOOD: reset plot colors

      EnterCriticalSection(&newTask.taskFinishedGuard);

      while ( newTask.callback != nullptr )
      {
        SleepConditionVariableCS(
          &newTask.taskFinished,
          &newTask.taskFinishedGuard,
          INFINITE ) != 0;
      }

      LeaveCriticalSection(&newTask.taskFinishedGuard);

      sorter->reset();
    }
    else
      Sleep(SortingStepPresentTime);

    isSorted = sorter->step();
  }

  return 0;
}

DWORD WINAPI
RandomizerThreadProc(
  LPVOID data )
{
  auto sharedState = static_cast <ThreadSharedData*> (data);

  auto& randomizerData = sharedState->randomizer;

  while ( AtomicLoadRelaxed(sharedState->sorterThreadsAreDead) == FALSE )
  {
    EnterCriticalSection(&randomizerData.taskAvailableGuard);

    while ( AtomicPointerLoadRelaxed(randomizerData.task) == nullptr &&
            AtomicLoadRelaxed(sharedState->sorterThreadsAreDead) == FALSE )
    {
      SleepConditionVariableCS(
        &randomizerData.taskAvailable,
        &randomizerData.taskAvailableGuard,
        INFINITE ) != 0;
    }

    LeaveCriticalSection(&randomizerData.taskAvailableGuard);


    auto task = static_cast <RandomizeTask*> (
      AtomicPointerExchangeAcquire(randomizerData.task, nullptr ) );

    if ( task != nullptr )
    {
      task->callback(
        task->data,
        task->elementCount,
        *task->dataGuard );

      EnterCriticalSection(&task->taskFinishedGuard);
        task->callback = nullptr; // mark task as done
      LeaveCriticalSection(&task->taskFinishedGuard);

      WakeConditionVariable(&task->taskFinished);
    }
  }

  return 0;
}


int
main()
{
  const size_t plotCount = 2;
  const size_t plotValueCount = 1000;
  using PlotValueType = uint32_t;
  using ThreadHandle = HANDLE;

  const auto heapMemoryBudget =
    sizeof(ThreadSharedData) +
    sizeof(PlotValueType) * plotValueCount * plotCount +
    sizeof(PlotValueColorIndex) * plotValueCount * plotCount +
    sizeof(ThreadLocalData) * plotCount +
    sizeof(ThreadHandle) * plotCount +
    sizeof(IAllocator*) * 100; // reserved for alignment padding & allocation bookkeeping

  ArenaAllocator arena {};
  arena.init(heapMemoryBudget);

  auto sharedState =
    ObjectCreate <ThreadSharedData> (arena);

  sharedState != nullptr;

  InitializeCriticalSection(&sharedState->randomizer.taskAvailableGuard);
  InitializeConditionVariable(&sharedState->randomizer.taskAvailable);


  struct PlotData
  {
    Array <PlotValueType> values {};
    Array <PlotValueColorIndex> colors {};
  };

  Array <PlotData> plotData {};
  plotData.init(plotCount, arena);

  for ( auto&& data : plotData )
  {
    data.values.init(plotValueCount, arena);
    data.colors.init(plotValueCount, arena);

    for ( size_t i {}; i < plotValueCount; ++i )
      data.values[i] = i;
  }


  Array <ThreadLocalData> threadsData {};

  threadsData.init(plotCount, arena, {*sharedState});

  threadsData[0].sorter =
    ObjectCreate <MockSorter <PlotValueType>> (
      arena,
      plotData[0].values,
      plotData[0].colors );

  threadsData[1].sorter =
    ObjectCreate <MockSorter <PlotValueType>> (
      arena,
      plotData[1].values,
      plotData[1].colors );


  Array <ThreadHandle> sorterThreads {};

  sorterThreads.init(plotCount, arena);

  for ( size_t i {}; i < plotCount; ++i )
  {
    sorterThreads[i] = CreateThread(
      NULL, 0,
      SorterThreadProc,
      &threadsData[i],
      0, 0 );

    sorterThreads[i] != NULL;
  }


  auto randomizerThread = CreateThread(
    NULL, 0,
    RandomizerThreadProc,
    sharedState,
    0, 0 );

  randomizerThread != NULL;


  for ( size_t frame {}; frame < 1000; ++frame )
  {
    for ( size_t i {}; i < plotCount; ++i )
    {
      if ( threadsData[i].sorter->tryLockData() == FALSE )
        continue;

//      simulate copying to back buffer
      Sleep(5);

      threadsData[i].sorter->unlockData();
    }

//    write to vertex buffer
//    glVertexAttribDivisor + glDrawArraysInstanced
//    swap
  }


  AtomicStoreRelaxed(
    sharedState->shutdownRequested, TRUE );

  for ( auto&& sorterThread : sorterThreads )
  {
    WaitForSingleObject(
      sorterThread, INFINITE ) != WAIT_FAILED;

    DWORD threadExitCode {};

    GetExitCodeThread(
      sorterThread, &threadExitCode ) != FALSE;

    threadExitCode != 0;
  }


  AtomicStoreRelaxed(
    sharedState->sorterThreadsAreDead, TRUE );

  EnterCriticalSection(&sharedState->randomizer.taskAvailableGuard);
  LeaveCriticalSection(&sharedState->randomizer.taskAvailableGuard);
  WakeConditionVariable(&sharedState->randomizer.taskAvailable);

  WaitForSingleObject(
    randomizerThread, INFINITE ) != WAIT_FAILED;

  DWORD threadExitCode {};

  GetExitCodeThread(
    randomizerThread, &threadExitCode ) != FALSE;

  threadExitCode != 0;


  for ( auto&& threadData : threadsData )
    ISorter::Destroy(threadData.sorter);

  DeleteCriticalSection(&sharedState->randomizer.taskAvailableGuard);
  ObjectDestroy(sharedState);


  return 0;
}
