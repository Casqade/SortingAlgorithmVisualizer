#include <SortingAlgorithmVisualizer/Containers.hpp>
#include <SortingAlgorithmVisualizer/CommonTypes.hpp>
#include <SortingAlgorithmVisualizer/Sorters/MockSorter.hpp>

#include <vector>
#include <thread>
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

      RandomizeTask* currentTask {};

      auto& newTask = sorter->getRandomizeTask();

      while ( randomizerData.task.compare_exchange_strong(
                currentTask, &newTask,
                std::memory_order_acq_rel,
                std::memory_order_relaxed ) == false )
        currentTask = nullptr;


      {
        std::unique_lock <std::mutex> lock {
          randomizerData.taskAvailableMutex };

        lock.unlock();
        randomizerData.taskAvailableSignal.notify_one();
      }


      std::unique_lock <std::mutex> lock (
        newTask.taskFinishedMutex );

      newTask.taskFinishedSignal.wait( lock,
        [&newTask]
        {
          return newTask.callback == nullptr;
        });

//      TOOD: reset plot colors

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
        return
          task.load(std::memory_order_relaxed) != nullptr ||
          sorterThreadsAreDead.load(std::memory_order_relaxed) == true;
      } );

    lock.unlock();

    auto task = randomizerData.task.load(std::memory_order_acquire);

    randomizerData.task.store(nullptr, std::memory_order_relaxed);

    if ( task != nullptr )
    {
      task->callback(task->data, task->elementCount);

      {
        std::unique_lock <std::mutex> lock (task->taskFinishedMutex);
        task->callback = nullptr; // mark task as done
        lock.unlock();
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

  ThreadSharedData sharedState {};

  std::vector <ThreadLocalData> threadData (
    plotCount, {sharedState} );

  PlotData <int> testData {};
  testData.init(1000);

  PlotData <int> testData1 {};
  testData1.init(1000);

  for ( size_t i {}; i < 1000; ++i )
  {
    testData[i] = i;
    testData1[i] = i;
  }

  threadData[0].sorter = new MockSorter <int> (testData);
  threadData[1].sorter = new MockSorter <int> (testData1);

  std::vector <std::thread> threads {};
  threads.reserve(plotCount);

  threads.emplace_back(sorterThreadProc, &threadData[0]);
  threads.emplace_back(sorterThreadProc, &threadData[1]);

  auto randomizerThread = std::thread(
    randomizerThreadProc, &sharedState );


  std::this_thread::sleep_for(std::chrono::seconds{1});

  sharedState.shutdownRequested.store(
    true, std::memory_order_relaxed );

  for ( auto&& thread : threads )
    thread.join();

  sharedState.sorterThreadsAreDead.store(
    true, std::memory_order_relaxed );


  {
    std::unique_lock <std::mutex> lock (
      sharedState.randomizer.taskAvailableMutex );

    sharedState.randomizer.taskAvailableSignal.notify_one();
  }

  randomizerThread.join();

  return 0;
}
