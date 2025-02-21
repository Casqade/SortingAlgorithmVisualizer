#pragma once

#include <SortingAlgorithmVisualizer/fwd.hpp>

#include <mutex>
#include <atomic>
#include <condition_variable>


using RandomizeFunction =
  void( void* data, size_t elementCount );


struct RandomizeTask
{
  std::mutex taskFinishedMutex {};
  std::condition_variable taskFinishedSignal {};

  RandomizeFunction* callback {};

  void* data {};
  size_t elementCount {};

  std::mutex* dataMutex {};
};


struct ThreadSharedData
{
  struct
  {
    std::mutex taskAvailableMutex {};
    std::condition_variable taskAvailableSignal {};

    std::atomic <RandomizeTask*> task {};

  } randomizer {};

  std::atomic_bool shutdownRequested {};
  std::atomic_bool sorterThreadsAreDead {};
};


struct ThreadLocalData
{
  ThreadSharedData& sharedState;
  ISorter* sorter {};
};
