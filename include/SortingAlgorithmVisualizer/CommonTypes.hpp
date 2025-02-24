#pragma once

#include <SortingAlgorithmVisualizer/fwd.hpp>

#include <windows.h>

#include <mutex>
#include <atomic>
#include <condition_variable>


using RandomizeFunction =
  void( void* data, size_t elementCount, CRITICAL_SECTION& dataGuard );


struct RandomizeTask
{
  std::mutex taskFinishedMutex {};
  std::condition_variable taskFinishedSignal {};

  RandomizeFunction* callback {};

  CRITICAL_SECTION* dataGuard {};
  void* data {};
  size_t elementCount {};
};


struct ThreadSharedData
{
  struct
  {
    std::mutex taskAvailableMutex {};
    std::condition_variable taskAvailableSignal {};

    RandomizeTask* task {};

  } randomizer {};

  std::atomic_bool shutdownRequested {};
  std::atomic_bool sorterThreadsAreDead {};
};


struct ThreadLocalData
{
  ThreadSharedData& sharedState;
  ISorter* sorter {};

  CRITICAL_SECTION dataGuard {};
};
