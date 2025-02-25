#pragma once

#include <SortingAlgorithmVisualizer/fwd.hpp>
#include <SortingAlgorithmVisualizer/Containers/Array.hpp>
#include <SortingAlgorithmVisualizer/Containers/RingBuffer.hpp>

#include <windows.h>

#include <cstdint>


using ThreadHandle = HANDLE;


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
    RingBuffer tasksIndices {};
    Array <RandomizeTask*> tasks {};

    CRITICAL_SECTION tasksAvailableGuard {};
    CONDITION_VARIABLE tasksAvailable {};

  } randomizer {};

  alignas(CacheLineSize)
  LONG shutdownRequested {};
  LONG sorterThreadsAreDead {};
};


struct ThreadLocalData
{
  ThreadSharedData& sharedState;
  ISorter* sorter {};
};


using PlotValueType = uint32_t;


enum class PlotValueColorIndex : uint8_t
{
  Unsorted,
  Sorted,

  MaxColors,
};


struct PlotData
{
  Array <PlotValueType> values {};
  Array <PlotValueColorIndex> colors {};
};
