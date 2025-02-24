#include <SortingAlgorithmVisualizer/Containers.hpp>
#include <SortingAlgorithmVisualizer/CommonTypes.hpp>
#include <SortingAlgorithmVisualizer/Sorters/MockSorter.hpp>
#include <SortingAlgorithmVisualizer/Allocators/ArenaAllocator.hpp>

#include <thread>
#include <cassert>
#include <condition_variable>


const auto SortingStepPresentTime =
  std::chrono::milliseconds{30};

const auto SortedResultPresentTime =
  std::chrono::seconds{3};

int32_t
sorterThreadProc(
  void* data )
{
  auto threadData = static_cast <ThreadLocalData*> (data);

  auto sorter = threadData->sorter;

  auto& sharedState = threadData->sharedState;

  bool isSorted {true};

  while ( sharedState.shutdownRequested.load(std::memory_order_relaxed) == false )
  {
    if ( isSorted == true )
    {
      std::this_thread::sleep_for(
        SortedResultPresentTime );


      auto& randomizerData = sharedState.randomizer;

      PVOID currentTask {};

      auto& newTask = sorter->getRandomizeTask();

      do
      {
        currentTask = InterlockedCompareExchangePointer(
          reinterpret_cast <PVOID*> (&randomizerData.task),
          &newTask, nullptr );

      } while ( currentTask != nullptr );


      {
        std::unique_lock <std::mutex> lock {
          randomizerData.taskAvailableMutex };

        lock.unlock();
        randomizerData.taskAvailableSignal.notify_one();
      }

//      TOOD: reset plot colors

      std::unique_lock <std::mutex> lock (
        newTask.taskFinishedMutex );

      newTask.taskFinishedSignal.wait( lock,
        [&newTask]
        {
          return newTask.callback == nullptr;
        });

      sorter->reset();
    }
    else
    {
      std::this_thread::sleep_for(
        SortingStepPresentTime );
    }

    isSorted = sorter->step();
  }

  return 0;
}

int32_t
randomizerThreadProc(
  void* data )
{
  auto sharedState = static_cast <ThreadSharedData*> (data);

  auto& randomizerData = sharedState->randomizer;

  while ( sharedState->sorterThreadsAreDead.load(std::memory_order_relaxed) == false )
  {
    std::unique_lock <std::mutex> lock (randomizerData.taskAvailableMutex);

    randomizerData.taskAvailableSignal.wait( lock,
      [&task = randomizerData.task, &sorterThreadsAreDead = sharedState->sorterThreadsAreDead]
      {
        auto taskPtr = InterlockedCompareExchangePointer(
          reinterpret_cast <PVOID*> (&task),
          nullptr, nullptr );

        return
          taskPtr != nullptr ||
          sorterThreadsAreDead.load(std::memory_order_relaxed) == true;
      } );

    lock.unlock();

    auto taskPtr = InterlockedCompareExchangePointer(
      reinterpret_cast <PVOID*> (&randomizerData.task),
      nullptr, nullptr );

    InterlockedCompareExchangePointer(
      reinterpret_cast <PVOID*> (&randomizerData.task),
      nullptr, taskPtr );

    auto task = static_cast <RandomizeTask*> (taskPtr);

    if ( task != nullptr )
    {
      task->callback(
        task->data,
        task->elementCount,
        *task->dataGuard );

      {
        std::lock_guard <std::mutex> lock (task->taskFinishedMutex);
        task->callback = nullptr; // mark task as done
      }

      task->taskFinishedSignal.notify_one();
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

  const auto heapMemoryBudget =
    sizeof(ThreadSharedData) +
    sizeof(PlotValueType) * plotValueCount * plotCount +
    sizeof(PlotValueColorIndex) * plotValueCount * plotCount +
    sizeof(ThreadLocalData) * plotCount +
    sizeof(std::thread) * plotCount +
    sizeof(IAllocator*) * 100; // reserved for alignment padding & allocation bookkeeping

  ArenaAllocator arena {};
  arena.init(heapMemoryBudget);

  auto sharedState =
    ObjectCreate <ThreadSharedData> (arena);

  assert(sharedState != nullptr);


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


  Array <std::thread> sorterThreads {};

  sorterThreads.init(plotCount, arena);

  for ( size_t i {}; i < plotCount; ++i )
    sorterThreads[i] = std::thread{
      sorterThreadProc, &threadsData[i] };


  auto randomizerThread = std::thread(
    randomizerThreadProc, sharedState );


  for ( size_t frame {}; frame < 1000; ++frame )
  {
    for ( size_t i {}; i < plotCount; ++i )
    {
      if ( threadsData[i].sorter->tryLockData() == false )
        continue;

//      simulate copying to back buffer
      std::this_thread::sleep_for(std::chrono::milliseconds{5});

      threadsData[i].sorter->unlockData();
    }

//    write to vertex buffer
//    glVertexAttribDivisor + glDrawArraysInstanced
//    swap
  }

  std::this_thread::sleep_for(std::chrono::seconds{1});


  sharedState->shutdownRequested.store(
    true, std::memory_order_relaxed );

  for ( auto&& thread : sorterThreads )
    thread.join();

  sharedState->sorterThreadsAreDead.store(
    true, std::memory_order_relaxed );


  {
    std::unique_lock <std::mutex> lock (
      sharedState->randomizer.taskAvailableMutex );

    sharedState->randomizer.taskAvailableSignal.notify_one();
  }

  randomizerThread.join();


  for ( auto&& threadData : threadsData )
    ISorter::Destroy(threadData.sorter);

  ObjectDestroy(sharedState);


  return 0;
}
