#pragma once

#include <SortingAlgorithmVisualizer/fwd.hpp>

#include <windows.h>

#include <cstdint>


using RandomizeFunction =
  void( void* data, size_t elementCount, CRITICAL_SECTION& dataGuard );


struct RandomizeTask
{
  CRITICAL_SECTION taskFinishedGuard {};
  CONDITION_VARIABLE taskFinished {};

  RandomizeFunction* callback {};

  CRITICAL_SECTION* dataGuard {};
  void* data {};
  size_t elementCount {};
};


struct ThreadSharedData
{
  struct
  {
    CRITICAL_SECTION taskAvailableGuard {};
    CONDITION_VARIABLE taskAvailable {};

    RandomizeTask* task {};

  } randomizer {};

  LONG shutdownRequested {};
  LONG sorterThreadsAreDead {};
};


struct ThreadLocalData
{
  ThreadSharedData& sharedState;
  ISorter* sorter {};
};


enum class PlotValueColorIndex : uint8_t
{
  Unsorted,
  Sorted,

  MaxColors,
};
