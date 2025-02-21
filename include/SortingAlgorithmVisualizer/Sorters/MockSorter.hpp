#pragma once

#include <SortingAlgorithmVisualizer/Sorters/ISorterSpecialized.hpp>


template <typename T>
class MockSorter : public ISorterSpecialized <T>
{
  size_t mCounter {};


public:
  inline MockSorter( PlotData <T>& data )
    : ISorterSpecialized <T>(data)
  {}

  ~MockSorter() override = default;


  inline void reset() override
  {
    mCounter = 0;
  }

  inline bool step() override
  {
    if ( this->mData == nullptr )
      return true;

    return ++mCounter == this->mData->size();
  }
};
