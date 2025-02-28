#pragma once

#include <SortingAlgorithmVisualizer/CommonTypes.hpp>
#include <SortingAlgorithmVisualizer/Randomization.hpp>
#include <SortingAlgorithmVisualizer/Sorters/ISorter.hpp>
#include <SortingAlgorithmVisualizer/Containers/Array.hpp>


template <typename T>
class ISorterSpecialized : public ISorter
{
public:
  ISorterSpecialized() = default;

  inline ~ISorterSpecialized()
  {
    mValues.deinit();
  }


  inline bool init( size_t valueCount, IAllocator& allocator ) override
  {
    if ( mValues.init(valueCount, allocator) == false )
    {
      MessageBox( NULL,
        "Failed to initialize plot values: "
        "Out of memory budget",
        NULL, MB_ICONERROR );

      ProgramShouldAbort = true;
      return false;
    }

    for ( size_t i {}; i < valueCount; ++i )
      mValues[i] = i;

    mRandomizeTask.data = mValues.data();
    mRandomizeTask.elementCount = valueCount;

    return ISorter::init(valueCount, allocator);
  }

  inline static size_t HeapMemoryBudget( size_t valueCount )
  {
    return
      sizeof(T) * valueCount +
      ISorter::HeapMemoryBudget(valueCount);
  }


protected:
  inline RandomizeFunction* getRandomizeCallback() const override
  {
    return RandomizePlotDataShuffle <T>;
  }


protected:
  Array <T> mValues {};
};
